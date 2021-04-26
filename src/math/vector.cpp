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

#include <math.h>
#include <ostream>

#include "math/vector.hpp"

#if 0

Vector Vector::unit() const
{
  return *this / norm();
}

Vector Vector::polar() const
{
  return x==0 && y==0 ? Vector(0,0) : Vector(norm(),angle());
}

Vector Vector::rectangular() const
{
  return Vector(cosf(y),sinf(y))*x;
}

float Vector::norm() const
{
  return sqrtf(x*x + y*y);
}

float Vector::angle() const
{
  return (x==0&&y==0) ? 0 : atan2f(y,x);
}

Vector Vector::at_angle(float rad) const
{
  return Vector(cosf(rad), sinf(rad)) * norm();
}

Vector Vector::rotate(float rad) const
{
  return at_angle(angle() + rad);
}

std::ostream& operator<<(std::ostream& out, const Vector& vector)
{
  out << "Vector(" << vector.x << ", " << vector.y << ')';
  return out;
}

#endif

/* EOF */
