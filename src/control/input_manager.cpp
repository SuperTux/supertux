//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007 Ingo Ruhnke <grumbel@gmx.de>
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

#include "control/input_manager.hpp"

#include <iostream>

#include "control/keyboard_manager.hpp"
#include "control/joystick_manager.hpp"
#include "control/game_controller_manager.hpp"
#include "gui/menu_manager.hpp"
#include "lisp/list_iterator.hpp"
#include "supertux/gameconfig.hpp"
#include "util/gettext.hpp"
#include "util/writer.hpp"

InputManager::InputManager() :
  controller(new Controller),
  m_use_game_controller(true),
  keyboard_manager(new KeyboardManager(this)),
  joystick_manager(new JoystickManager(this)),
  game_controller_manager(new GameControllerManager(this))
{
}

InputManager::~InputManager()
{
}

Controller*
InputManager::get_controller()
{
  return controller.get();
}

void
InputManager::read(const Reader& lisp)
{
  const lisp::Lisp* keymap_lisp = lisp.get_lisp("keymap");
  if (keymap_lisp) 
  {
    keyboard_manager->read(keymap_lisp);
  }

  const lisp::Lisp* joystick_lisp = lisp.get_lisp(_("joystick"));
  if (joystick_lisp) 
  {
    joystick_manager->read(joystick_lisp);
  }
}

void
InputManager::write(Writer& writer)
{
  writer.start_list("keymap");
  keyboard_manager->write(writer);
  writer.end_list("keymap");

  writer.start_list("joystick");
  joystick_manager->write(writer);
  writer.end_list("joystick");
}

void
InputManager::update()
{
  controller->update();
}

void
InputManager::reset()
{
  controller->reset();
}

void
InputManager::process_event(const SDL_Event& event)
{
  switch(event.type) {
    case SDL_TEXTINPUT:
      keyboard_manager->process_text_input_event(event.text);
      break;

    case SDL_KEYUP:
    case SDL_KEYDOWN:
      keyboard_manager->process_key_event(event.key);
      break;

    case SDL_JOYAXISMOTION:
      if (!m_use_game_controller) joystick_manager->process_axis_event(event.jaxis);
      break;

    case SDL_JOYHATMOTION:
      if (!m_use_game_controller) joystick_manager->process_hat_event(event.jhat);
      break;

    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      if (!m_use_game_controller) joystick_manager->process_button_event(event.jbutton);
      break;

    case SDL_JOYDEVICEADDED:
      if (!m_use_game_controller) joystick_manager->on_joystick_added(event.jdevice.which);
      break;

    case SDL_JOYDEVICEREMOVED:
      if (!m_use_game_controller) joystick_manager->on_joystick_removed(event.jdevice.which);
      break;

    case SDL_CONTROLLERAXISMOTION:
      if (m_use_game_controller) game_controller_manager->process_axis_event(event.caxis);
      break;

    case SDL_CONTROLLERBUTTONDOWN:
      if (m_use_game_controller) game_controller_manager->process_button_event(event.cbutton);
      break;

    case SDL_CONTROLLERBUTTONUP:
      if (m_use_game_controller) game_controller_manager->process_button_event(event.cbutton);
      break;

    case SDL_CONTROLLERDEVICEADDED:
      std::cout << "SDL_CONTROLLERDEVICEADDED" << std::endl;
      if (m_use_game_controller) game_controller_manager->on_controller_added(event.cdevice.which);
      break;

    case SDL_CONTROLLERDEVICEREMOVED:
      std::cout << "SDL_CONTROLLERDEVICEREMOVED" << std::endl;
      if (m_use_game_controller) game_controller_manager->on_controller_removed(event.cdevice.which);
      break;

    case SDL_CONTROLLERDEVICEREMAPPED:
      std::cout << "SDL_CONTROLLERDEVICEREMAPPED" << std::endl;
      break;

    default:
      break;
  }
}

/* EOF */
