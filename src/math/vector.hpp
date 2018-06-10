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
#include <cmath>
#include "util/log.hpp"
/** Simple two dimensional vector. */
class Vector
{
public:
  Vector(float nx, float ny)
    : x(nx), y(ny)
  { }
  Vector(const Vector& other)
    : x(other.x), y(other.y)
  { }
  Vector()
    : x(0), y(0)
  { }

  bool operator ==(const Vector& other) const
  {
    return x == other.x && y == other.y;
  }

  bool operator !=(const Vector& other) const
  {
    return !(x == other.x && y == other.y);
  }

  Vector& operator=(const Vector& other)
  {
    x = other.x;
    y = other.y;
    return *this;
  }

  Vector operator+(const Vector& other) const
  {
    return Vector(x + other.x, y + other.y);
  }

  Vector operator-(const Vector& other) const
  {
    return Vector(x - other.x, y - other.y);
  }

  Vector operator*(float s) const
  {
    return Vector(x * s, y * s);
  }

  Vector operator/(float s) const
  {
    return Vector(x / s, y / s);
  }

  Vector operator-() const
  {
    return Vector(-x, -y);
  }

  const Vector& operator +=(const Vector& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  const Vector& operator -=(const Vector& other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  const Vector& operator *=(float val)
  {
    x *= val;
    y *= val;
    return *this;
  }

  const Vector& operator /=(float val)
  {
    x /= val;
    y /= val;
    return *this;
  }

  /// Scalar product of 2 vectors
  float operator*(const Vector& other) const
  {
    return x*other.x + y*other.y;
  }

  float norm() const;
  Vector unit() const;

  Vector to_int_vec() const
  {
    return Vector(int(x), int(y));
  }
  /// Checks if two vectors are colinear (i.e returns true iff \exists k \in \mathbb R : k*other = this)
  bool is_colinear(const Vector& other) const
  {
    if(other.x == 0 && x != 0)
      return false;
    if(y == 0 && other.y != 0)
      return false;
    double val = 0;
    if(other.x != 0)
      val = other.x/x;
    else 
      val = other.y/y;
    Vector v = Vector(val*x, val*y);
    double abstand = abs(v.x-other.x)+abs(v.y-other.y);
    // Allow for floating point errors
    return abstand <= 0.002;
  
  }
  /// Returns a vector perpendicular to this vector
  Vector perp() const
  {
    Vector v;
    v.x = y;
    v.y = -x;
    return v;
  }
  // ... add the other operators as needed, I'm too lazy now ...

  float x, y; // leave this public, get/set methods just give me headaches
  // for such simple stuff :)
};

#endif

/* EOF */
