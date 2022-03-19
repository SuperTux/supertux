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

#include "config.h"


#include "math/rectf.hpp"
#include "video/surface_ptr.hpp"

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
  bool m_up, m_down, m_left, m_right, m_jump, m_action, m_cheats, m_debug, m_escape;
  bool m_old_up, m_old_down, m_old_left, m_old_right, m_old_jump, m_old_action, m_old_cheats, m_old_debug, m_old_escape;

  const Rectf m_rect_directions, m_rect_jump, m_rect_action, m_rect_cheats,
              m_rect_debug, m_rect_escape;
  const SurfacePtr m_tex_dirs, m_tex_btn, m_tex_btn_press, m_tex_pause,
                   m_tex_up, m_tex_dwn, m_tex_lft, m_tex_rgt,
                   m_tex_jump, m_tex_action, m_tex_cheats, m_tex_debug;

  int m_screen_width, m_screen_height;

private:
  MobileController(const MobileController&) = delete;
  MobileController& operator=(const MobileController&) = delete;
};

#endif

/* EOF */
