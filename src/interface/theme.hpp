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

#ifndef HEADER_SUPERTUX_INTERFACE_THEME_HPP
#define HEADER_SUPERTUX_INTERFACE_THEME_HPP

#include "video/color.hpp"
#include "video/font_ptr.hpp"

class UITheme
{
public:
  UITheme(Color bg, Color bg_hover, Color bg_active, Color bg_focus,
          Color txt, Color txt_hover, Color txt_active, Color txt_focus,
          FontPtr f, float box_radius, float text_height_correction) :
    bg_color(bg),
    bg_hover_color(bg_hover),
    bg_active_color(bg_active),
    bg_focus_color(bg_focus),
    tx_color(txt),
    tx_hover_color(txt_hover),
    tx_active_color(txt_active),
    tx_focus_color(txt_focus),
    font(f),
    radius(box_radius),
    txt_hgt(text_height_correction)
  {
  }

  Color bg_color, bg_hover_color, bg_active_color, bg_focus_color,
        tx_color, tx_hover_color, tx_active_color, tx_focus_color;
  FontPtr font;
  float radius;
  float txt_hgt;
};

#endif

/* EOF */
