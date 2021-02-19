//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_MATH_TRIANGLE3D_HPP
#define HEADER_SUPERTUX_MATH_TRIANGLE3D_HPP

#include "math/triangle.hpp"
#include "math/vector3.hpp"

class Triangle3D final
{
public:
  Triangle3D() : p1(), p2(), p3() {}
  Triangle3D(const Vector3& p1_, const Vector3& p2_, const Vector3& p3_) :
    p1(p1_),
    p2(p2_),
    p3(p3_)
  {
  }

  bool is_above(const Triangle3D& vector) const;

  /** Moves the whole triangle by this vector */
  Triangle3D moved(const Vector3& d) const { return Triangle3D(p1 + d, p2 + d, p3 + d); }

  /** Shifts the extrudes/intrudes of the depthmap towards the given point */
  Triangle shifted(const Vector& p) const
  {
    return Triangle(p + (p1.get_2d() - p) * p1.z,
                    p + (p2.get_2d() - p) * p2.z,
                    p + (p3.get_2d() - p) * p3.z);
  }

  Triangle flattened() const
  {
    return Triangle(p1.get_2d(), p2.get_2d(), p3.get_2d());
  }

public:
  Vector3 p1, p2, p3;
};

std::ostream& operator<<(std::ostream& out, const Triangle3D& vector);

#endif

/* EOF */
