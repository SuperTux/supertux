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
  Color() :
    red(0), 
    green(0), 
    blue(0), 
    alpha(1.0f)
  {}

  Color(float red_, float green_, float blue_, float alpha_ = 1.0) :
    red(red_),
    green(green_), 
    blue(blue_), 
    alpha(alpha_)
  {
    assert(0 <= red   && red <= 1.0);
    assert(0 <= green && green <= 1.0);
    assert(0 <= blue  && blue <= 1.0);
  }

  Color(const std::vector<float>& vals) :
    red(),
    green(),
    blue(),
    alpha()
  {
    assert(vals.size() >= 3);
    red   = vals[0];
    green = vals[1];
    blue  = vals[2];
    if(vals.size() > 3)
      alpha = vals[3];
    else
      alpha = 1.0;
    assert(0 <= red   && red <= 1.0);
    assert(0 <= green && green <= 1.0);
    assert(0 <= blue  && blue <= 1.0);
  }

  bool operator==(const Color& other) const
  {
    return red == other.red && green == other.green && blue == other.blue
      && alpha == other.alpha;
  }

  float greyscale() const
  {
    return red * 0.30 + green * 0.59 + blue * 0.11;
  }

  bool operator < (const Color& other) const
  {
    return greyscale() < other.greyscale();
  }

  float red, green, blue, alpha;

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
