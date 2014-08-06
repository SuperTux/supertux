//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_CONTROL_JOYSTICKKEYBOARDCONTROLLER_HPP
#define HEADER_SUPERTUX_CONTROL_JOYSTICKKEYBOARDCONTROLLER_HPP

#include "control/controller.hpp"

#include <SDL.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"

class Menu;
class KeyboardMenu;
class JoystickMenu;
class Controller;
class JoystickManager;

class JoystickKeyboardController
{
private:
  friend class KeyboardMenu;
  friend class JoystickMenu;

  typedef Controller::Control Control;

  typedef std::map<SDL_Keycode, Control> KeyMap;

public:
  JoystickKeyboardController();
  virtual ~JoystickKeyboardController();

  /** Process an SDL Event and return true if the event has been used
   */
  void process_event(const SDL_Event& event);

  void write(Writer& writer);
  void read(const Reader& lisp);
  void update();
  void reset();

  void updateAvailableJoysticks();

  Controller *get_main_controller();

private:
  void process_text_input_event(const SDL_TextInputEvent& event);
  void process_key_event(const SDL_KeyboardEvent& event);
  void process_console_key_event(const SDL_KeyboardEvent& event);
  void process_menu_key_event(const SDL_KeyboardEvent& event);

  SDL_Keycode reversemap_key(Control c);
  void bind_key(SDL_Keycode key, Control c);

private:
  std::unique_ptr<Controller> controller;
public:
  std::unique_ptr<JoystickManager> joystick_manager;

private:
  KeyMap keymap;

  std::string name;

  bool jump_with_up_kbd; // Keyboard up jumps

  int wait_for_key;

private:
  JoystickKeyboardController(const JoystickKeyboardController&);
  JoystickKeyboardController& operator=(const JoystickKeyboardController&);
};

#endif

/* EOF */
