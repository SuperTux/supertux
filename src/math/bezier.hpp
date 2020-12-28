//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_MATH_BEZIER_HPP
#define HEADER_SUPERTUX_MATH_BEZIER_HPP

#include <math/vector.hpp>

class Color;
class DrawingContext;

class Bezier
{
public:
  // p1 is first anchor, p2 is first handle, p3 is second handle, p4 is second anchor. T is progress from p1 towards p4.
  static Vector get_point(Vector p1, Vector p2, Vector p3, Vector p4, float t);
  // FIXME: Move this to the Canvas object?
  static void draw_curve(DrawingContext& context, Vector p1, Vector p2, Vector p3, Vector p4, int steps, Color color, int layer);

private:
  Bezier(const Bezier&) = delete;
  Bezier& operator=(const Bezier&) = delete;
};

#endif

/* EOF */
