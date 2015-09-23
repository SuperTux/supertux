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

#ifndef HEADER_SUPERTUX_CONTROL_JOYSTICK_CONFIG_HPP
#define HEADER_SUPERTUX_CONTROL_JOYSTICK_CONFIG_HPP

#include <map>
#include <SDL.h>

#include "control/controller.hpp"
#include "lisp/lisp.hpp"
#include "util/writer.hpp"

class JoystickConfig
{
public:
  typedef Uint8 JoyId;
  typedef std::map<std::pair<JoyId, int>, Controller::Control> ButtonMap;
  typedef std::map<std::pair<JoyId, int>, Controller::Control> AxisMap;
  typedef std::map<std::pair<JoyId, int>, Controller::Control> HatMap;

public:
  int dead_zone;
  bool jump_with_up_joy;

  ButtonMap joy_button_map;
  AxisMap joy_axis_map;
  HatMap joy_hat_map;

public:
  JoystickConfig();

  void print_joystick_mappings() const;

  int reversemap_joybutton(Controller::Control c) const;
  int reversemap_joyaxis(Controller::Control c) const;
  int reversemap_joyhat(Controller::Control c) const;

  void unbind_joystick_control(Controller::Control c);

  void bind_joybutton(JoyId joy_id, int button, Controller::Control c);
  void bind_joyaxis(JoyId joy_id, int axis, Controller::Control c);
  void bind_joyhat(JoyId joy_id, int dir, Controller::Control c);

  void read(const lisp::Lisp& joystick_lisp);
  void write(Writer& writer);

private:
  JoystickConfig(const JoystickConfig&) = delete;
  JoystickConfig& operator=(const JoystickConfig&) = delete;
};

#endif

/* EOF */
