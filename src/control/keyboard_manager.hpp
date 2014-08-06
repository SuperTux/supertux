//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007-2014 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_CONTROL_KEYBOARD_MANAGER_HPP
#define HEADER_SUPERTUX_CONTROL_KEYBOARD_MANAGER_HPP

#include <map>

#include "SDL.h"

#include "control/controller.hpp"
#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"

class InputManager;

class KeyboardManager final
{
private:
  friend class KeyboardMenu;
  typedef std::map<SDL_Keycode, Controller::Control> KeyMap;

public:
  KeyboardManager(InputManager* parent);
  ~KeyboardManager();

  void process_key_event(const SDL_KeyboardEvent& event);
  void process_text_input_event(const SDL_TextInputEvent& event);
  void process_console_key_event(const SDL_KeyboardEvent& event);
  void process_menu_key_event(const SDL_KeyboardEvent& event);

  SDL_Keycode reversemap_key(Controller::Control c);
  void bind_key(SDL_Keycode key, Controller::Control c);

  void read(const lisp::Lisp* keymap_lisp);
  void write(Writer& writer);

private:
  InputManager* m_parent;
  KeyMap keymap;
  bool jump_with_up_kbd;
  int wait_for_key;

private:
  KeyboardManager(const KeyboardManager&) = delete;
  KeyboardManager& operator=(const KeyboardManager&) = delete;
};

#endif

/* EOF */
