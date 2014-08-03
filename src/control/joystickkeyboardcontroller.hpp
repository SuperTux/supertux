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
#include <SDL_keycode.h> // add by giby
#include <map>
#include <string>
#include <vector>

#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"

class Menu;
class KeyboardMenu;
class JoystickMenu;
class Controller;

    //SDL_JoystickID myID = SDL_JoystickInstanceID(myOpenedStick);

class JoystickKeyboardController // http://wiki.libsdl.org/moin.fcg/SDL_Joystick for info 
{
private:
  friend class KeyboardMenu;
  friend class JoystickMenu;

  typedef Controller::Control Control;
  typedef Uint8 JoyId;

  typedef std::map<SDL_Keycode, Control> KeyMap;
  typedef std::map<std::pair<JoyId, int>, Control> ButtonMap;
  typedef std::map<std::pair<JoyId, int>, Control> AxisMap;
  typedef std::map<std::pair<JoyId, int>, Control> HatMap;

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
  void process_hat_event(const SDL_JoyHatEvent& jhat);
  void process_axis_event(const SDL_JoyAxisEvent& jaxis);
  void process_button_event(const SDL_JoyButtonEvent& jbutton);
  void process_console_key_event(const SDL_KeyboardEvent& event);
  void process_menu_key_event(const SDL_KeyboardEvent& event);

  void print_joystick_mappings();

  SDL_Keycode reversemap_key(Control c);
  int    reversemap_joybutton(Control c);
  int    reversemap_joyaxis(Control c);
  int    reversemap_joyhat(Control c);

  void unbind_joystick_control(Control c);

  void bind_joybutton(JoyId joy_id, int button, Control c);
  void bind_joyaxis(JoyId joy_id, int axis, Control c);
  void bind_joyhat(JoyId joy_id, int dir, Control c);
  void bind_key(SDL_Keycode key, Control c);

  void set_joy_controls(Control id, bool value); 

private:
  Controller *controller;

  KeyMap keymap;

  ButtonMap joy_button_map;

  AxisMap joy_axis_map;

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

  int wait_for_key;
  int wait_for_joystick;

private:
  JoystickKeyboardController(const JoystickKeyboardController&);
  JoystickKeyboardController& operator=(const JoystickKeyboardController&);
};

#endif

/* EOF */
