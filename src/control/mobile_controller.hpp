//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_CONTROL_MOBILE_CONTROLLER_HPP
#define HEADER_SUPERTUX_CONTROL_MOBILE_CONTROLLER_HPP

#include "math/rectf.hpp"

class Controller;
class DrawingContext;

class MobileController final
{
public:
  MobileController();
  void draw(DrawingContext& context);
  void apply(Controller& controller) const;
  void update();

private:
  void activate_widget_at_pos(float x, float y);

private:
  bool m_up, m_down, m_left, m_right;
  bool m_jump, m_action, m_escape;

  const Rectf m_rect_directions, m_rect_jump, m_rect_action, m_rect_escape;

  int m_screen_width, m_screen_height;

private:
  MobileController(const MobileController&) = delete;
  MobileController& operator=(const MobileController&) = delete;
};

#endif

/* EOF */
