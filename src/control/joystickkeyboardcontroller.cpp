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

#include "control/joystickkeyboardcontroller.hpp"

#include <iostream>

#include "control/joystick_manager.hpp"
#include "control/game_controller_manager.hpp"
#include "gui/menu_manager.hpp"
#include "lisp/list_iterator.hpp"
#include "supertux/console.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"
#include "util/writer.hpp"

JoystickKeyboardController::JoystickKeyboardController() :
  controller(new Controller),
  m_use_game_controller(true),
  joystick_manager(new JoystickManager(this)),
  game_controller_manager(new GameControllerManager(this)),
  keymap(),
  jump_with_up_kbd(),
  wait_for_key(-1)
{
  // initialize default keyboard map
  keymap[SDLK_LEFT]     = Controller::LEFT;
  keymap[SDLK_RIGHT]    = Controller::RIGHT;
  keymap[SDLK_UP]       = Controller::UP;
  keymap[SDLK_DOWN]     = Controller::DOWN;
  keymap[SDLK_SPACE]    = Controller::JUMP;
  keymap[SDLK_LCTRL]    = Controller::ACTION;
  keymap[SDLK_LALT]     = Controller::ACTION;
  keymap[SDLK_ESCAPE]   = Controller::PAUSE_MENU;
  keymap[SDLK_p]        = Controller::PAUSE_MENU;
  keymap[SDLK_PAUSE]    = Controller::PAUSE_MENU;
  keymap[SDLK_RETURN]   = Controller::MENU_SELECT;
  keymap[SDLK_KP_ENTER] = Controller::MENU_SELECT;
  keymap[SDLK_CARET]    = Controller::CONSOLE;
  keymap[SDLK_DELETE]   = Controller::PEEK_LEFT;
  keymap[SDLK_PAGEDOWN] = Controller::PEEK_RIGHT;
  keymap[SDLK_HOME]     = Controller::PEEK_UP;
  keymap[SDLK_END]      = Controller::PEEK_DOWN;

  jump_with_up_kbd = false;
}

JoystickKeyboardController::~JoystickKeyboardController()
{
}

Controller*
JoystickKeyboardController::get_main_controller()
{
  return controller.get();
}

void
JoystickKeyboardController::read(const Reader& lisp)
{
  const lisp::Lisp* keymap_lisp = lisp.get_lisp("keymap");
  if (keymap_lisp) {
    keymap.clear();
    keymap_lisp->get("jump-with-up", jump_with_up_kbd);
    lisp::ListIterator iter(keymap_lisp);
    while(iter.next()) {
      if (iter.item() == "map") {
        int key = -1;
        std::string control;
        const lisp::Lisp* map = iter.lisp();
        map->get("key", key);
        map->get("control", control);

        int i = 0;
        for(i = 0; Controller::controlNames[i] != 0; ++i) {
          if (control == Controller::controlNames[i])
            break;
        }
        if (Controller::controlNames[i] == 0) {
          log_info << "Invalid control '" << control << "' in keymap" << std::endl;
          continue;
        }
        keymap[SDL_Keycode(key)] = Control(i);
      }
    }
  }

  const lisp::Lisp* joystick_lisp = lisp.get_lisp(_("joystick"));
  if (joystick_lisp) 
  {
    joystick_manager->read(joystick_lisp);
  }
}

void
JoystickKeyboardController::write(Writer& writer)
{
  writer.start_list("keymap");
  writer.write("jump-with-up", jump_with_up_kbd);
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i) {
    writer.start_list("map");
    writer.write("key", (int) i->first);
    writer.write("control", Controller::controlNames[i->second]);
    writer.end_list("map");
  }
  writer.end_list("keymap");

  writer.start_list("joystick");
  joystick_manager->write(writer);
  writer.end_list("joystick");
}

void
JoystickKeyboardController::update()
{
  controller->update();
}

void
JoystickKeyboardController::reset()
{
  controller->reset();
}

void
JoystickKeyboardController::process_event(const SDL_Event& event)
{
  switch(event.type) {
    case SDL_TEXTINPUT:
      process_text_input_event(event.text);
      break;

    case SDL_KEYUP:
    case SDL_KEYDOWN:
      process_key_event(event.key);
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

void
JoystickKeyboardController::process_text_input_event(const SDL_TextInputEvent& event)
{
  if (Console::instance->hasFocus()) {
    for(int i = 0; event.text[i] != '\0'; ++i)
    {
      Console::instance->input(event.text[i]);
    }
  }
}

void
JoystickKeyboardController::process_key_event(const SDL_KeyboardEvent& event)
{
  KeyMap::iterator key_mapping = keymap.find(event.keysym.sym);

  // if console key was pressed: toggle console
  if ((key_mapping != keymap.end()) && (key_mapping->second == Controller::CONSOLE)) {
    if (event.type == SDL_KEYDOWN) 
      Console::instance->toggle();
  } else {
    if (Console::instance->hasFocus()) {
      // if console is open: send key there
      process_console_key_event(event);
    } else if (MenuManager::current()) {
      // if menu mode: send key there
      process_menu_key_event(event);
    } else if (key_mapping == keymap.end()) {
      // default action: update controls
      //log_debug << "Key " << event.key.SDL_Keycode.sym << " is unbound" << std::endl;
    } else {
      Control control = key_mapping->second;
      bool value = (event.type == SDL_KEYDOWN);
      controller->set_control(control, value);
      if (jump_with_up_kbd && control == Controller::UP){
        controller->set_control(Controller::JUMP, value);
      }
    }
  }
}

void
JoystickKeyboardController::process_console_key_event(const SDL_KeyboardEvent& event)
{
  if (event.type != SDL_KEYDOWN) return;

  switch (event.keysym.sym) {
    case SDLK_RETURN:
      Console::instance->enter();
      break;
    case SDLK_BACKSPACE:
      Console::instance->backspace();
      break;
    case SDLK_TAB:
      Console::instance->autocomplete();
      break;
    case SDLK_PAGEUP:
      Console::instance->scroll(-1);
      break;
    case SDLK_PAGEDOWN:
      Console::instance->scroll(+1);
      break;
    case SDLK_HOME:
      Console::instance->move_cursor(-65535);
      break;
    case SDLK_END:
      Console::instance->move_cursor(+65535);
      break;
    case SDLK_UP:
      Console::instance->show_history(-1);
      break;
    case SDLK_DOWN:
      Console::instance->show_history(+1);
      break;
    case SDLK_LEFT:
      Console::instance->move_cursor(-1);
      break;
    case SDLK_RIGHT:
      Console::instance->move_cursor(+1);
      break;
    default:
      break;
  }
}

void
JoystickKeyboardController::process_menu_key_event(const SDL_KeyboardEvent& event)
{
  // wait for key mode?
  if (wait_for_key >= 0) {
    if (event.type == SDL_KEYUP)
      return;

    if (event.keysym.sym != SDLK_ESCAPE
       && event.keysym.sym != SDLK_PAUSE) {
      bind_key(event.keysym.sym, Control(wait_for_key));
    }
    reset();
    MenuStorage::get_key_options_menu()->update();
    wait_for_key = -1;
    return;
  }
  if (joystick_manager->wait_for_joystick >= 0) {
    if (event.keysym.sym == SDLK_ESCAPE) {
      reset();
      MenuStorage::get_joystick_options_menu()->update();
      joystick_manager->wait_for_joystick = -1;
    }
    return;
  }

  Control control;
  /* we use default keys when the menu is open (to avoid problems when
   * redefining keys to invalid settings
   */
  switch(event.keysym.sym) {
    case SDLK_UP:
      control = Controller::UP;
      break;
    case SDLK_DOWN:
      control = Controller::DOWN;
      break;
    case SDLK_LEFT:
      control = Controller::LEFT;
      break;
    case SDLK_RIGHT:
      control = Controller::RIGHT;
      break;
    case SDLK_SPACE:
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      control = Controller::MENU_SELECT;
      break;
    case SDLK_ESCAPE:
    case SDLK_PAUSE:
      control = Controller::PAUSE_MENU;
      break;
    default:
      return;
      break;
  }

  controller->set_control(control, (event.type == SDL_KEYDOWN));
}

void
JoystickKeyboardController::bind_key(SDL_Keycode key, Control control)
{
  // remove all previous mappings for that control and for that key
  for(KeyMap::iterator i = keymap.begin();
      i != keymap.end(); /* no ++i */) {
    if (i->second == control) {
      KeyMap::iterator e = i;
      ++i;
      keymap.erase(e);
    } else {
      ++i;
    }
  }

  KeyMap::iterator i = keymap.find(key);
  if (i != keymap.end())
    keymap.erase(i);

  // add new mapping
  keymap[key] = control;
}

SDL_Keycode
JoystickKeyboardController::reversemap_key(Control c)
{
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i) {
    if (i->second == c)
      return i->first;
  }

  return SDLK_UNKNOWN;
}

/* EOF */
