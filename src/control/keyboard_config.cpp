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

#include "control/keyboard_config.hpp"

#include "util/log.hpp"
#include "util/reader_mapping.hpp"

KeyboardConfig::KeyboardConfig() :
  keymap(),
  jump_with_up_kbd(false)
{
  // initialize default keyboard map
  keymap[SDLK_LEFT]     = Controller::LEFT;
  keymap[SDLK_RIGHT]    = Controller::RIGHT;
  keymap[SDLK_UP]       = Controller::UP;
  keymap[SDLK_DOWN]     = Controller::DOWN;
  keymap[SDLK_SPACE]    = Controller::JUMP;
  keymap[SDLK_LCTRL]    = Controller::ACTION;
  keymap[SDLK_LALT]     = Controller::ACTION;
  keymap[SDLK_ESCAPE]   = Controller::ESCAPE;
  keymap[SDLK_p]        = Controller::START;
  keymap[SDLK_PAUSE]    = Controller::START;
  keymap[SDLK_RETURN]   = Controller::MENU_SELECT;
  keymap[SDLK_KP_ENTER] = Controller::MENU_SELECT;
  keymap[SDLK_CARET]    = Controller::CONSOLE;
  keymap[SDLK_DELETE]   = Controller::PEEK_LEFT;
  keymap[SDLK_PAGEDOWN] = Controller::PEEK_RIGHT;
  keymap[SDLK_HOME]     = Controller::PEEK_UP;
  keymap[SDLK_END]      = Controller::PEEK_DOWN;
  keymap[SDLK_F1]       = Controller::CHEAT_MENU;
}

void
KeyboardConfig::read(const ReaderMapping& keymap_lisp)
{
  // keycode values changed between SDL1 and SDL2, so we skip old SDL1
  // based values and use the defaults instead on the first read of
  // the config file
  bool config_is_sdl2 = false;
  keymap_lisp.get("sdl2", config_is_sdl2);
  if (config_is_sdl2)
  {
    keymap_lisp.get("jump-with-up", jump_with_up_kbd);

    auto iter = keymap_lisp.get_iter();
    while(iter.next())
    {
      if (iter.get_key() == "map")
      {
        int key = -1;
        std::string control;
        auto map = iter.as_mapping();
        map.get("key", key);
        map.get("control", control);

        int i = 0;
        for(i = 0; Controller::controlNames[i] != 0; ++i)
        {
          if (control == Controller::controlNames[i])
            break;
        }

        if (Controller::controlNames[i] == 0)
        {
          log_info << "Invalid control '" << control << "' in keymap" << std::endl;
          continue;
        }
        keymap[static_cast<SDL_Keycode>(key)] = static_cast<Controller::Control>(i);
      }
    }
  }
}

void
KeyboardConfig::bind_key(SDL_Keycode key, Controller::Control control)
{
  // remove all previous mappings for that control and for that key
  for(KeyMap::iterator i = keymap.begin();
      i != keymap.end();
      /* no ++i */)
  {
    if (i->second == control)
    {
      KeyMap::iterator e = i;
      ++i;
      keymap.erase(e);
    }
    else
    {
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
KeyboardConfig::reversemap_key(Controller::Control c) const
{
  for(const auto& i : keymap)
  {
    if (i.second == c)
    {
      return i.first;
    }
  }

  return SDLK_UNKNOWN;
}

void
KeyboardConfig::write(Writer& writer)
{
  // this flag handles the transition from SDL1 to SDL2, as keycodes
  // are incompatible between the two, if it's not set an old SDL1
  // config file is assumed and controls are reset to default
  writer.write("sdl2", true);

  writer.write("jump-with-up", jump_with_up_kbd);

  for(const auto& i : keymap)
  {
    writer.start_list("map");
    writer.write("key", (int) i.first);
    writer.write("control", Controller::controlNames[i.second]);
    writer.end_list("map");
  }
}

/* EOF */
