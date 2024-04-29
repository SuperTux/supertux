//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#ifndef HEADER_SUPERTUX_VIDEO_COLORSPACE_HPP
#define HEADER_SUPERTUX_VIDEO_COLORSPACE_HPP

#include <map>

#include "math/circle.hpp"
#include "math/rectf.hpp"
#include "video/color.hpp"

/** A virtual colorspace, consisting of colored shapes,
    where the color at a provided point can be gathered deterministically. */
class ColorSpace final
{
public:
  enum Type
  {
    NONE,
    LIGHTSPRITES /* Colorspace of object light sprites */
  };

public:
  ColorSpace();

  void add(const Rectf& rect, const Color& color);
  void add(const Circle& circle, const Color& color);

  Color get_pixel(const Vector& point) const;

private:
  std::map<Color, Rectf> m_rects;
  std::map<Color, Circle> m_circles;
};

#endif

/* EOF */
