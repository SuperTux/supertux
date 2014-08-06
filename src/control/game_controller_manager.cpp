//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "control/game_controller_manager.hpp"

#include <algorithm>

#include "control/joystickkeyboardcontroller.hpp"
#include "util/log.hpp"

GameControllerManager::GameControllerManager(JoystickKeyboardController* parent) :
  m_parent(parent),
  m_deadzone(8000),
  m_game_controllers()
{
}

GameControllerManager::~GameControllerManager()
{
  for(auto con : m_game_controllers)
  {
    SDL_GameControllerClose(con);
  }
}

void
GameControllerManager::process_button_event(const SDL_ControllerButtonEvent& ev)
{
  //log_info << "button event: " << static_cast<int>(ev.button) << " " << static_cast<int>(ev.state) << std::endl;
  auto controller = m_parent->get_main_controller();
  switch(ev.button)
  {
    case SDL_CONTROLLER_BUTTON_A:
      controller->set_control(Controller::JUMP, ev.state);
      controller->set_control(Controller::MENU_SELECT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_B:
      controller->set_control(Controller::MENU_BACK, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_X:
      controller->set_control(Controller::ACTION, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_Y:
      break;

    case SDL_CONTROLLER_BUTTON_BACK:
      break;

    case SDL_CONTROLLER_BUTTON_GUIDE:
      controller->set_control(Controller::CONSOLE, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_START:
      controller->set_control(Controller::PAUSE_MENU, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
      break;

    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
      break;

    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
      controller->set_control(Controller::PEEK_LEFT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
      controller->set_control(Controller::PEEK_RIGHT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_UP:
      controller->set_control(Controller::UP, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      controller->set_control(Controller::DOWN, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
      controller->set_control(Controller::LEFT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
      controller->set_control(Controller::RIGHT, ev.state);
      break;

    default:
      break;
  }
}

void
GameControllerManager::process_axis_event(const SDL_ControllerAxisEvent& ev)
{
  // FIXME: buttons and axis are fighting for control ownership, need
  // to OR the values together

  //log_info << "axis event: " << static_cast<int>(ev.axis) << " " << ev.value << std::endl;
  auto controller = m_parent->get_main_controller();
  auto axis2button = [this, &controller](int value,
                                         Controller::Control control_left, Controller::Control control_right)
    {
      if (value < -m_deadzone)
      {
        controller->set_control(control_left, true);
        controller->set_control(control_right, false);
      }
      else if (value > m_deadzone)
      {
        controller->set_control(control_left, false);
        controller->set_control(control_right, true);
      }
      else
      {
        controller->set_control(control_left, false);
        controller->set_control(control_right, false);
      }
    };

  switch(ev.axis)
  {
    case SDL_CONTROLLER_AXIS_LEFTX:
      axis2button(ev.value, Controller::LEFT, Controller::RIGHT);
      break;

    case SDL_CONTROLLER_AXIS_LEFTY:
      axis2button(ev.value, Controller::UP, Controller::DOWN);
      break;

    case SDL_CONTROLLER_AXIS_RIGHTX:
      axis2button(ev.value, Controller::PEEK_LEFT, Controller::PEEK_RIGHT);
      break;

    case SDL_CONTROLLER_AXIS_RIGHTY:
      axis2button(ev.value, Controller::PEEK_UP, Controller::PEEK_DOWN);
      break;

    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
      break;

    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
      break;

    default:
      break;
  }
}

void
GameControllerManager::on_controller_added(int joystick_index)
{
  if (!SDL_IsGameController(joystick_index))
  {
    log_warning << "joystick is not a game controller, ignoring: " << joystick_index << std::endl;
  }
  else
  {
    SDL_GameController* game_controller = SDL_GameControllerOpen(joystick_index);
    if (!game_controller)
    {
      log_warning << "failed to open game_controller: " << joystick_index
                  << ": " << SDL_GetError() << std::endl;
    }
    else
    {
      m_game_controllers.push_back(game_controller);
    }
  }
}

void
GameControllerManager::on_controller_removed(int instance_id)
{
  for(auto& controller : m_game_controllers)
  {
    SDL_Joystick* joy = SDL_GameControllerGetJoystick(controller);
    SDL_JoystickID id = SDL_JoystickInstanceID(joy);
    if (id == instance_id)
    {
      SDL_GameControllerClose(controller);
      controller = nullptr;
    }
  }

  m_game_controllers.erase(std::remove(m_game_controllers.begin(), m_game_controllers.end(), nullptr),
                           m_game_controllers.end());
}

/* EOF */
