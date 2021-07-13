//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
//                2006 Matthias Braun <matze@braunis.de> (see vector.cpp)
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

#include <math.h>
#include <ostream>

#include "math/vector3.hpp"

Vector3 Vector3::unit() const
{
  return *this / norm();
}

float Vector3::norm() const
{
  return sqrtf(x*x + y*y + z*z);
}

std::ostream& operator<<(std::ostream& out, const Vector3& v)
{
  out << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ')';
  return out;
}

/* EOF */
