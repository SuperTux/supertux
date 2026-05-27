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

#pragma once

#include <map>
#include <SDL.h>

#include "control/controller.hpp"

class ReaderMapping;
class Writer;

class JoystickConfig final
{
  friend class InputManager;
  friend class JoystickManager;
  friend class JoystickMenu;

public:
  JoystickConfig();

  void print_joystick_mappings() const;

  int reversemap_joybutton(Control c) const;
  int reversemap_joyaxis(Control c) const;
  int reversemap_joyhat(Control c) const;

  void unbind_joystick_control(Control c);

  void bind_joybutton(int button, Control c);
  void bind_joyaxis(int axis, Control c);
  void bind_joyhat(int dir, Control c);

  void read(const ReaderMapping& joystick_mapping);
  void write(Writer& writer);

private:
  int m_dead_zone;
  bool m_jump_with_up_joy;

  std::map<int, Control> m_joy_button_map;
  std::map<int, Control> m_joy_axis_map;
  std::map<int, Control> m_joy_hat_map;
};
