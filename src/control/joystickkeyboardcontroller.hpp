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

namespace lisp {
class Writer;
class Lisp;
}

#include <SDL.h>

#include <map>
#include <string>
#include <vector>

class Menu;

class JoystickKeyboardController : public Controller
{
public:
  JoystickKeyboardController();
  virtual ~JoystickKeyboardController();

  /** Process an SDL Event and return true if the event has been used
   */
  void process_event(const SDL_Event& event);

  void write(lisp::Writer& writer);
  void read(const lisp::Lisp& lisp);
  void reset();

  Menu* get_key_options_menu();
  Menu* get_joystick_options_menu();
  void updateAvailableJoysticks();

private:
  void process_key_event(const SDL_Event& event);
  void process_hat_event(const SDL_JoyHatEvent& jhat);
  void process_axis_event(const SDL_JoyAxisEvent& jaxis);
  void process_button_event(const SDL_JoyButtonEvent& jbutton);
  void process_console_key_event(const SDL_Event& event);
  void process_menu_key_event(const SDL_Event& event);

  void print_joystick_mappings();

  typedef std::map<SDLKey, Control> KeyMap;
  KeyMap keymap;

  typedef std::map<int, Control> ButtonMap;
  ButtonMap joy_button_map;

  typedef std::map<int, Control> AxisMap;
  AxisMap joy_axis_map;

  typedef std::map<int, Control> HatMap;
  HatMap joy_hat_map;

  std::vector<SDL_Joystick*> joysticks;

  std::string name;

  int dead_zone;
  /// the number of buttons all joysticks have
  int min_joybuttons;
  /// the max number of buttons a joystick has
  int max_joybuttons;

  int max_joyaxis;

  int max_joyhats;

  Uint8 hat_state;

  bool jump_with_up_joy; // Joystick up jumps
  bool jump_with_up_kbd; // Keyboard up jumps

  SDLKey reversemap_key(Control c);
  int    reversemap_joybutton(Control c);
  int    reversemap_joyaxis(Control c);
  int    reversemap_joyhat(Control c);

  void unbind_joystick_control(Control c);

  void bind_joybutton(int button, Control c);
  void bind_joyaxis(int axis, Control c);
  void bind_joyhat(int dir, Control c);
  void bind_key(SDLKey key, Control c);

  void set_joy_controls(Control id, bool value);

  int wait_for_key;
  int wait_for_joystick;

  class KeyboardMenu;
  class JoystickMenu;

  KeyboardMenu* key_options_menu;
  JoystickMenu* joystick_options_menu;
  friend class KeyboardMenu;
  friend class JoystickMenu;

private:
  JoystickKeyboardController(const JoystickKeyboardController&);
  JoystickKeyboardController& operator=(const JoystickKeyboardController&);
};

#endif

/* EOF */
