//  SuperTux
//  Copyright (C) 2024 bruhmoent
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

#ifndef HEADER_SUPERTUX_GUI_COLOR_CLIPBOARD_HPP
#define HEADER_SUPERTUX_GUI_COLOR_CLIPBOARD_HPP

#include <memory>

#include "video/color.hpp"

class ColorClipboard 
{
public:
  ColorClipboard();
  ~ColorClipboard() = default;

  static ColorClipboard& instance();

  void set_color(const Color& color);
  const Color* get_color() const;

private:
  std::unique_ptr<Color> m_color;

private:
  ColorClipboard(const ColorClipboard&) = delete;
  ColorClipboard& operator=(const ColorClipboard&) = delete;
};

#endif

/* EOF */
