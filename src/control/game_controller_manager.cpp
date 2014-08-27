//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/input_manager.hpp"
#include "util/log.hpp"

GameControllerManager::GameControllerManager(InputManager* parent) :
  m_parent(parent),
  m_deadzone(8000),
  m_game_controllers(),
  m_stick_state(),
  m_button_state()
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
  auto controller = m_parent->get_controller();
  auto set_control = [this, &controller](Controller::Control control, bool value)
  {
    m_button_state[control] = value;
    controller->set_control(control, m_button_state[control] || m_stick_state[control]);
  };
  switch(ev.button)
  {
    case SDL_CONTROLLER_BUTTON_A:
      set_control(Controller::JUMP, ev.state);
      set_control(Controller::MENU_SELECT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_B:
      set_control(Controller::MENU_BACK, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_X:
      set_control(Controller::ACTION, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_Y:
      break;

    case SDL_CONTROLLER_BUTTON_BACK:
      set_control(Controller::CONSOLE, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_GUIDE:
      set_control(Controller::CHEAT_MENU, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_START:
      set_control(Controller::START, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
      break;

    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
      break;

    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
      set_control(Controller::PEEK_LEFT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
      set_control(Controller::PEEK_RIGHT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_UP:
      set_control(Controller::UP, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      set_control(Controller::DOWN, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
      set_control(Controller::LEFT, ev.state);
      break;

    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
      set_control(Controller::RIGHT, ev.state);
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
  auto controller = m_parent->get_controller();
  auto set_control = [this, &controller](Controller::Control control, bool value)
  {
    m_stick_state[control] = value;
    controller->set_control(control, m_button_state[control] || m_stick_state[control]);
  };

  auto axis2button = [this, &set_control](int value,
                                         Controller::Control control_left, Controller::Control control_right)
    {
      if (value < -m_deadzone)
      {
        set_control(control_left, true);
        set_control(control_right, false);
      }
      else if (value > m_deadzone)
      {
        set_control(control_left, false);
        set_control(control_right, true);
      }
      else
      {
        set_control(control_left, false);
        set_control(control_right, false);
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
