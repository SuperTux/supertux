//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_VIDEO_COLOR_HPP
#define HEADER_SUPERTUX_VIDEO_COLOR_HPP

#include <assert.h>
#include <vector>

#include "util/log.hpp"

class Color
{
public:
  Color();

  Color(float red_, float green_, float blue_, float alpha_ = 1.0, float ultra_violet_ = 0.0);

  Color(const std::vector<float>& vals);

  /**
   * Composes the colour again.
   */
  Color(const Color visible, const Color hidden);

  bool operator==(const Color& other) const;

  float greyscale() const;

  bool operator < (const Color& other) const;

  /**
   * Returns the colour that should be rendered on the visible lightmap.
   */
  Color get_visible_color() const;

  /**
   * Returns the colour that should be rendered on the hidden lightmap.
   */
  Color get_hidden_color() const;

  std::vector<float> toVector();

  float red, green, blue, alpha, ultra_violet;

  static const Color BLACK;
  static const Color RED;
  static const Color GREEN;
  static const Color BLUE;
  static const Color CYAN;
  static const Color MAGENTA;
  static const Color YELLOW;
  static const Color WHITE;
};

#endif

/* EOF */
