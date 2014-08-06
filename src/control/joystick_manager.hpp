//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_CONTROL_JOYSTICK_MANAGER_HPP
#define HEADER_SUPERTUX_CONTROL_JOYSTICK_MANAGER_HPP

#include <map>
#include <vector>

#include "SDL.h"

#include "control/controller.hpp"
#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"

class InputManager;

class JoystickManager final
{
private:
  typedef Uint8 JoyId;

  typedef std::map<std::pair<JoyId, int>, Controller::Control> ButtonMap;
  typedef std::map<std::pair<JoyId, int>, Controller::Control> AxisMap;
  typedef std::map<std::pair<JoyId, int>, Controller::Control> HatMap;

private:
  InputManager* parent;

  ButtonMap joy_button_map;
  AxisMap joy_axis_map;
  HatMap joy_hat_map;

  int dead_zone;

  /// the number of buttons all joysticks have
  int min_joybuttons;

  /// the max number of buttons a joystick has
  int max_joybuttons;

  int max_joyaxis;
  int max_joyhats;

  Uint8 hat_state;

public:
  bool jump_with_up_joy;

public:
  int wait_for_joystick;

public:
  std::vector<SDL_Joystick*> joysticks;

public:
  JoystickManager(InputManager* parent);
  ~JoystickManager();

  void process_hat_event(const SDL_JoyHatEvent& jhat);
  void process_axis_event(const SDL_JoyAxisEvent& jaxis);
  void process_button_event(const SDL_JoyButtonEvent& jbutton);

  void print_joystick_mappings();

  int reversemap_joybutton(Controller::Control c);
  int reversemap_joyaxis(Controller::Control c);
  int reversemap_joyhat(Controller::Control c);

  void unbind_joystick_control(Controller::Control c);

  void bind_joybutton(JoyId joy_id, int button, Controller::Control c);
  void bind_joyaxis(JoyId joy_id, int axis, Controller::Control c);
  void bind_joyhat(JoyId joy_id, int dir, Controller::Control c);

  void set_joy_controls(Controller::Control id, bool value);

  void on_joystick_added(int joystick_index);
  void on_joystick_removed(int instance_id);

  void read(const lisp::Lisp* joystick_lisp);
  void write(Writer& writer);

private:
  JoystickManager(const JoystickManager&) = delete;
  JoystickManager& operator=(const JoystickManager&) = delete;
};

#endif

/* EOF */
