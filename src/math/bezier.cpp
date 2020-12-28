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

#include <math/bezier.hpp>
#include <video/color.hpp>
#include <video/drawing_context.hpp>

Vector
Bezier::get_point(Vector p1, Vector p2, Vector p3, Vector p4, float t)
{
  float t2 = 1.f - t;

  return p1 * (t2 * t2 * t2) + p2 * t2 * t2 * t * 3.f  + p3 * t2 * t * t * 3.f + p4 * (t * t * t);
}

void
Bezier::draw_curve(DrawingContext& context, Vector p1, Vector p2, Vector p3, Vector p4, int steps, Color color, int layer)
{
  for (int i = 0; i < steps; i += 1)
  {
    float f1 = static_cast<float>(i) / static_cast<float>(steps),
          f2 = static_cast<float>(i + 1) / static_cast<float>(steps);

    context.color().draw_line(get_point(p1, p2, p3, p4, f1),
                              get_point(p1, p2, p3, p4, f2),
                              color,
                              layer);
  }
}

/* EOF */
