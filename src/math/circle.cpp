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

#include "math/circle.hpp"

#include "math/rectf.hpp"
#include "util/reader_mapping.hpp"

Circle
Circle::from_reader(const ReaderMapping& mapping)
{
  Circle circle;

  mapping.get("radius", circle.m_radius);

  return circle;
}


Circle::Circle(const Vector& center, float radius) :
  m_center(center),
  m_radius(radius)
{
}

bool
Circle::contains(const Vector& point) const
{
  const float distance = powf(point.x - m_center.x, 2) + powf(point.y - m_center.y, 2);
  return distance <= powf(m_radius, 2);
}

/* EOF */
