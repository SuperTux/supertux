//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
//                2023 Vankata453
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

#ifndef HEADER_SUPERTUX_GUI_MENU_TRANSITION_HPP
#define HEADER_SUPERTUX_GUI_MENU_TRANSITION_HPP

#include "math/rectf.hpp"

class DrawingContext;

class MenuTransition final
{
private:
  Rectf m_from_rect;
  Rectf m_to_rect;

  float m_effect_progress;
  float m_effect_start_time;
  bool m_is_active;

public:
  MenuTransition();

  void start(const Rectf& from_rect, const Rectf& to_rect);
  void update();
  void draw(DrawingContext& context);

  void set(const Rectf& rect) { m_to_rect = m_from_rect = rect; }
  bool is_active() const { return m_is_active; }
};

#endif

/* EOF */
