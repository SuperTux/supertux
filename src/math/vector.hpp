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
#include <iosfwd>

#include "math/util.hpp"

#include "glm/glm.hpp"

/** Simple two dimensional vector. */
class Vector final
{
public:
  Vector(float nx, float ny): x(nx), y(ny) { }
  Vector(const Vector& other): x(other.x), y(other.y) { }
  Vector(): x(0), y(0) { }

  static Vector from_angle(float angle);

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

  Vector operator+(float other) const
  {
    return Vector(x + other, y + other);
  }

  Vector operator-(float other) const
  {
    return Vector(x - other, y - other);
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

  const Vector& operator +=(float other)
  {
    x += other;
    y += other;
    return *this;
  }

  const Vector& operator -=(float other)
  {
    x -= other;
    y -= other;
    return *this;
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

  /// Element-wise product of 2 vectors
  Vector operator^(const Vector& other) const
  {
    return Vector(x*other.x, y*other.y);
  }

  Vector normalize() const;
  float length() const;
  float angle() const;
  Vector floor() const;
  Vector polar() const;
  Vector rectangular() const;
  Vector trunc() const;
  float distance(const Vector& other) const;


  // ... add the other operators as needed, I'm too lazy now ...

  float x, y; // leave this public, get/set methods just give me headaches
  // for such simple stuff :)
};

std::ostream& operator<<(std::ostream& out, const Vector& vector);

#endif

/* EOF */
