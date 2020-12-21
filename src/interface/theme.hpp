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
  UITheme(Color bg, Color bg_focus, Color txt, FontPtr f) :
    bg_color(bg),
    bg_focus_color(bg_focus),
    txt_color(txt),
    font(f)
  {
  }

  Color bg_color, bg_focus_color, txt_color;
  FontPtr font;
};

#endif

/* EOF */
