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

#include "math/vector.hpp"

#include <cmath>
#include <ostream>

Vector
Vector::polar() const
{
  return x==0 && y==0 ? Vector(0,0) : Vector(length(),angle());
}

Vector
Vector::rectangular() const
{
  return Vector(cosf(y),sinf(y))*x;
}

Vector
Vector::trunc() const
{
  return Vector(truncf(x), truncf(y));
}

float
Vector::distance(const Vector& other) const
{
  return sqrtf((other.x - x)*2 + (other.y - y)*2);
}

Vector
Vector::floor() const
{
  return Vector(floorf(x), floorf(y));
}

Vector
Vector::from_angle(float angle)
{
  return Vector(std::cos(angle), std::sin(angle));
}

Vector
Vector::normalize() const
{
  return *this / length();
}

float
Vector::length() const
{
  return sqrtf(x*x + y*y);
}

float
Vector::angle() const
{
  return (x==0&&y==0) ? 0 : atan2(y,x);
}

std::ostream&
operator<<(std::ostream& out, const Vector& vector)
{
  out << "(" << vector.x << ", " << vector.y << ')';
  return out;
}

/* EOF */

