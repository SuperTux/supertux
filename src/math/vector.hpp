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

#include "fwd.hpp"

namespace math {

template<typename T>
inline Vector_t<T> vec2_from_polar(T length, T angle)
{
  return Vector_t<T>(cosf(angle), sinf(angle)) * length;
}

template<typename T>
inline T angle(Vector_t<T> const& v)
{
  return (v.x == 0 && v.y == 0) ? 0 : atan2f(v.y, v.x);
}

template<typename T>
inline Vector_t<T> at_angle(Vector_t<T> const& v, T angle)
{
  return vec2_from_polar(glm::length(v), angle);
}

} // namespace math

#endif

/* EOF */
