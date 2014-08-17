//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007-2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/keyboard_manager.hpp"

#include "control/controller.hpp"
#include "control/joystick_manager.hpp"
#include "gui/menu_manager.hpp"
#include "lisp/list_iterator.hpp"
#include "supertux/console.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/writer.hpp"

KeyboardManager::KeyboardManager(InputManager* parent) :
  m_parent(parent),
  keymap(),
  jump_with_up_kbd(false),
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
  keymap[SDLK_TAB]      = Controller::CHEAT_MENU;
}

KeyboardManager::~KeyboardManager()
{
}

void
KeyboardManager::process_key_event(const SDL_KeyboardEvent& event)
{
  KeyMap::iterator key_mapping = keymap.find(event.keysym.sym);

  // if console key was pressed: toggle console
  if (key_mapping != keymap.end() &&
      key_mapping->second == Controller::CONSOLE)
  {
    if (event.type == SDL_KEYDOWN)
    {
      Console::current()->toggle();
    }
  }
  else if (Console::current()->hasFocus())
  {
    // if console is open: send key there
    process_console_key_event(event);
  }
  else if (MenuManager::instance().is_active())
  {
    // if menu mode: send key there
    process_menu_key_event(event);
  }
  else if (key_mapping == keymap.end())
  {
    // default action: update controls
    //log_debug << "Key " << event.key.SDL_Keycode.sym << " is unbound" << std::endl;
  }
  else
  {
    auto control = key_mapping->second;
    bool value = (event.type == SDL_KEYDOWN);
    m_parent->get_controller()->set_control(control, value);
    if (jump_with_up_kbd && control == Controller::UP)
    {
      m_parent->get_controller()->set_control(Controller::JUMP, value);
    }
  }
}

void
KeyboardManager::process_text_input_event(const SDL_TextInputEvent& event)
{
  if (Console::current()->hasFocus()) {
    for(int i = 0; event.text[i] != '\0'; ++i)
    {
      Console::current()->input(event.text[i]);
    }
  }
}

void
KeyboardManager::process_console_key_event(const SDL_KeyboardEvent& event)
{
  if (event.type != SDL_KEYDOWN) return;

  switch (event.keysym.sym) {
    case SDLK_RETURN:
      Console::current()->enter();
      break;
    case SDLK_BACKSPACE:
      Console::current()->backspace();
      break;
    case SDLK_TAB:
      Console::current()->autocomplete();
      break;
    case SDLK_PAGEUP:
      Console::current()->scroll(-1);
      break;
    case SDLK_PAGEDOWN:
      Console::current()->scroll(+1);
      break;
    case SDLK_HOME:
      Console::current()->move_cursor(-65535);
      break;
    case SDLK_END:
      Console::current()->move_cursor(+65535);
      break;
    case SDLK_UP:
      Console::current()->show_history(-1);
      break;
    case SDLK_DOWN:
      Console::current()->show_history(+1);
      break;
    case SDLK_LEFT:
      Console::current()->move_cursor(-1);
      break;
    case SDLK_RIGHT:
      Console::current()->move_cursor(+1);
      break;
    default:
      break;
  }
}

void
KeyboardManager::process_menu_key_event(const SDL_KeyboardEvent& event)
{
  // wait for key mode?
  if (wait_for_key >= 0)
  {
    if (event.type == SDL_KEYUP)
      return;

    if (event.keysym.sym != SDLK_ESCAPE &&
        event.keysym.sym != SDLK_PAUSE)
    {
      bind_key(event.keysym.sym, static_cast<Controller::Control>(wait_for_key));
    }
    m_parent->reset();
    MenuManager::instance().refresh();
    wait_for_key = -1;
    return;
  }

  if (m_parent->joystick_manager->wait_for_joystick >= 0)
  {
    if (event.keysym.sym == SDLK_ESCAPE)
    {
      m_parent->reset();
      MenuManager::instance().refresh();
      m_parent->joystick_manager->wait_for_joystick = -1;
    }
    return;
  }

  Controller::Control control;
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

  m_parent->get_controller()->set_control(control, (event.type == SDL_KEYDOWN));
}

void
KeyboardManager::bind_key(SDL_Keycode key, Controller::Control control)
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
KeyboardManager::reversemap_key(Controller::Control c)
{
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i)
  {
    if (i->second == c)
    {
      return i->first;
    }
  }

  return SDLK_UNKNOWN;
}

void
KeyboardManager::read(const lisp::Lisp* keymap_lisp)
{
  // keycode values changed between SDL1 and SDL2, so we skip old SDL1
  // based values and use the defaults instead on the first read of
  // the config file
  bool config_is_sdl2 = false;
  keymap_lisp->get("sdl2", config_is_sdl2);
  if (config_is_sdl2)
  {
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
        keymap[static_cast<SDL_Keycode>(key)] = static_cast<Controller::Control>(i);
      }
    }
  }
}

void
KeyboardManager::write(Writer& writer)
{
  writer.write("sdl2", true);
  writer.write("jump-with-up", jump_with_up_kbd);
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i) {
    writer.start_list("map");
    writer.write("key", (int) i->first);
    writer.write("control", Controller::controlNames[i->second]);
    writer.end_list("map");
  }
}

/* EOF */
