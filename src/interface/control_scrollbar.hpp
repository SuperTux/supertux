//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_INTERFACE_CONTROL_SCROLLBAR_HPP
#define HEADER_SUPERTUX_INTERFACE_CONTROL_SCROLLBAR_HPP

#include "editor/widget.hpp"

#include "math/rectf.hpp"
#include "math/vector.hpp"

class DrawingContext;
union SDL_Event;

/** A generic template for a scrollbar */
class ControlScrollbar final : public Widget
{
public:
  ControlScrollbar();

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;

private:
  /** Whether or not the mouse is clicking on the bar */
  bool m_scrolling;

  /** Whether or not the mouse hovers above the bar */
  bool m_hovering;

  /** The length (height) of the region to scroll */
  int m_total_region;

  /** The length (height) of the viewport for the region */
  int m_covered_region;

  /** The length (height) between the beginning of the viewport and the beginning of the region */
  int m_progress;
  
  /** The logical position and size of the widget */
  Rectf m_rect;
  
  /** `true` of the scroller is horizontal; `false` if it is vertical */
  //bool is_horizontal;

private:
  Rectf get_bar_rect();

  float last_mouse_pos;
  //float zoom_factor;

private:
  ControlScrollbar(const ControlScrollbar&) = delete;
  ControlScrollbar& operator=(const ControlScrollbar&) = delete;
};

#endif

/* EOF */
