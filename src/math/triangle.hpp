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

#ifndef HEADER_SUPERTUX_MATH_TRIANGLE_HPP
#define HEADER_SUPERTUX_MATH_TRIANGLE_HPP

#include "math/vector.hpp"

class Triangle final
{
public:
  Triangle() : p1(), p2(), p3() {}
  Triangle(const Vector& p1_, const Vector& p2_, const Vector& p3_) :
    p1(p1_),
    p2(p2_),
    p3(p3_)
  {
  }

  /** Moves the whole triangle by this vector */
  Triangle moved(const Vector& d) const { return Triangle(p1 + d, p2 + d, p3 + d); }

public:
  Vector p1, p2, p3;
};

std::ostream& operator<<(std::ostream& out, const Triangle& vector);

#endif

/* EOF */
