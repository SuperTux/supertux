//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "control/controller.hpp"

/**
 * Class responsible for editor event handling (keyboard, mouse, etc.)
 */
class EditorEventHandling
{
public:
  EditorEventHandling();

  void on_event(const SDL_Event& ev);
  void reset_state();
  void update_keyboard(const Controller& controller);

  Vector get_mouse_pos() const { return m_mouse_pos; } 
  bool get_ctrl_pressed() const { return m_ctrl_pressed; }
  bool get_shift_pressed() const { return m_shift_pressed; }
  bool get_alt_pressed() const { return m_alt_pressed; }
  bool get_key_zoomed_pressed() const { return m_key_zoomed; }
  void set_key_zoomed_pressed(bool key_zoomed_pressed) { m_key_zoomed = key_zoomed_pressed; }

  bool get_pen_down() const { return m_pen_down; }

private:
  Vector m_mouse_pos;
  bool m_ctrl_pressed;
  bool m_shift_pressed;
  bool m_alt_pressed;
  bool m_key_zoomed;
  bool m_pen_down;
};