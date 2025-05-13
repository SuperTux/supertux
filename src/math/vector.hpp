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

#ifndef HEADER_SUPERTUX_MATH_VECTOR_HPP
#define HEADER_SUPERTUX_MATH_VECTOR_HPP

#include <math.h>
#include <iosfwd>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>

using Vector = glm::vec2;

namespace math {

inline Vector vec2_from_polar(float length, float angle)
{
  return Vector(cosf(angle), sinf(angle)) * length;
}

inline float angle(Vector const& v)
{
  return (v.x == 0 && v.y == 0) ? 0 : atan2f(v.y, v.x);
}

inline Vector at_angle(Vector const& v, float angle)
{
  return vec2_from_polar(glm::length(v), angle);
}

// Move vector towards a new vector by a scalar delta.
inline Vector move_towards(Vector const& from, Vector const& to, float d) {
  // Based on Godot's implementation
  Vector vd = to - from;
  float len = vd.length();
  return len <= d ? to : from + vd / len * d;
}

// Change a velocity vector towards another, but do not change a component towards zero unless their signs are opposite.
inline Vector push_to_velocity(Vector const& from, Vector const& to, float d) {
  if (d == 0.f) return from;

  Vector diff = glm::normalize(to - from) * d;
  Vector result = from;

  if (to.x > 0 && from.x < to.x)
    result.x = std::min(from.x + diff.x, to.x);
  if (to.x < 0 && from.x > to.x)
    result.x = std::max(from.x + diff.x, to.x);
  if (to.y > 0 && from.y < to.y)
    result.y = std::min(from.y + diff.y, to.y);
  if (to.y < 0 && from.y > to.y)
    result.y = std::max(from.y + diff.y, to.y);

  return result;
}

} // namespace math

#endif

/* EOF */
