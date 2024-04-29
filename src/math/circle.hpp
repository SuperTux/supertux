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

#ifndef HEADER_SUPERTUX_MATH_CIRCLE_HPP
#define HEADER_SUPERTUX_MATH_CIRCLE_HPP

#include "math/vector.hpp"

class Rectf;

class Circle final
{
public:
  Circle(const Vector& center, float radius = 0.f);

  bool contains(const Vector& point) const;
  //bool overlaps(const Rectf& rect) const;

  const Vector& get_center() const { return m_center; }

private:
  Vector m_center;
  float m_radius;
};

#endif

/* EOF */
