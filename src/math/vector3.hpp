//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
//                2006 Matthias Braun <matze@braunis.de> (see vector.hpp)
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

#ifndef HEADER_SUPERTUX_MATH_VECTOR3_HPP
#define HEADER_SUPERTUX_MATH_VECTOR3_HPP

#include <math.h>
#include <iosfwd>

#include "math/util.hpp"
#include "math/vector.hpp"

/** Simple three dimensional vector. NOTE: The Z axis is a SCALE, not an OFFSET! */
class Vector3 final
{
public:
  Vector3(float nx, float ny, float nz)
    : x(nx), y(ny), z(nz)
  { }
  Vector3(const Vector3& other)
    : x(other.x), y(other.y), z(other.z)
  { }
  Vector3(const Vector& other)
    : x(other.x), y(other.y), z(0.f)
  { }
  Vector3(const Vector& other, float z_)
    : x(other.x), y(other.y), z(z_)
  { }
  Vector3()
    : x(0), y(0), z(0)
  { }

  Vector get_2d() const { return Vector(x, y); }

  bool operator ==(const Vector3& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }

  bool operator !=(const Vector3& other) const
  {
    return !(x == other.x && y == other.y && z == other.z);
  }

  Vector3& operator=(const Vector3& other)
  {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }

  Vector3 operator+(const Vector3& other) const
  {
    return Vector3(x + other.x, y + other.y, z + other.z);
  }

  Vector3 operator-(const Vector3& other) const
  {
    return Vector3(x - other.x, y - other.y, z - other.z);
  }

  Vector3 operator*(float s) const
  {
    return Vector3(x * s, y * s, z * s);
  }

  Vector3 operator/(float s) const
  {
    return Vector3(x / s, y / s, z / s);
  }

  Vector3 operator%(float s) const
  {
    return Vector3(x - floorf(x / s) * s, y - floorf(y / s) * s, z - floorf(y / s) * s);
  }

  Vector3 operator-() const
  {
    return Vector3(-x, -y, -z);
  }

  const Vector3& operator +=(const Vector3& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  const Vector3& operator -=(const Vector3& other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  const Vector3& operator *=(float val)
  {
    x *= val;
    y *= val;
    z *= val;
    return *this;
  }

  const Vector3& operator /=(float val)
  {
    x /= val;
    y /= val;
    z /= val;
    return *this;
  }

  /// Scalar product of 2 vectors
  float operator*(const Vector3& other) const
  {
    return x*other.x + y*other.y + z*other.z;
  }

  float norm() const;
  Vector3 unit() const;

  Vector3 floor() const
  {
    return Vector3(floorf(x), floorf(y), floorf(z));
  }

  // ... add the other operators as needed, I'm too lazy now ...

  float x, y, z; // leave this public, get/set methods just give me headaches
  // for such simple stuff :)
};

std::ostream& operator<<(std::ostream& out, const Vector3& vector);

#endif

/* EOF */
