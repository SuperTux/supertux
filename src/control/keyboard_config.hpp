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

#ifndef HEADER_SUPERTUX_CONTROL_KEYBOARD_CONFIG_HPP
#define HEADER_SUPERTUX_CONTROL_KEYBOARD_CONFIG_HPP

#include <SDL.h>
#include <map>

#include "control/controller.hpp"

class ReaderMapping;
class Writer;

class KeyboardConfig
{
public:
  KeyboardConfig(int playernumber = 0);

  SDL_Keycode reversemap_key(Controller::Control c) const;
  void bind_key(SDL_Keycode key, Controller::Control c);

  void read(const ReaderMapping& keymap_lisp);
  void write(Writer& writer);

  typedef std::map<SDL_Keycode, Controller::Control> KeyMap;
  KeyMap keymap;
  bool jump_with_up_kbd;
  int playernum;
};

#endif

/* EOF */
