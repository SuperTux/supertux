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

#include "video/colorspace.hpp"

ColorSpace::ColorSpace() :
  m_rects(),
  m_circles()
{
}

void
ColorSpace::add(const Rectf& rect, const Color& color)
{
  m_rects.emplace_back(rect, color);
}

void
ColorSpace::add(const Circle& circle, const Color& color)
{
  m_circles.emplace_back(circle, color);
}

Color
ColorSpace::get_pixel(const Vector& point) const
{
  Color result(0.f, 0.f, 0.f);
  
  for (const auto& rect : m_rects)
  {
    if (rect.shape.contains(point))
      result = (result + rect.color).validate(); // Additive blending
  }
  for (const auto& circle : m_circles)
  {
    if (circle.shape.contains(point))
      result = (result + circle.color).validate(); // Additive blending
  }

  return result;
}

/* EOF */
