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

#ifndef HEADER_SUPERTUX_COLLISION_COLLISION_HPP
#define HEADER_SUPERTUX_COLLISION_COLLISION_HPP

#include <limits>
#include <algorithm>

#include "collision/collision_hit.hpp"
#include "math/fwd.hpp"
#include "constraints.hpp"

class Rectf;
class AATriangle;

namespace collision {

inline Vector make_normal(const Vector& p1, const Vector& p2);

/** does collision detection between a rectangle and an axis aligned triangle
 * Returns true in case of a collision and fills in the hit structure then.
 */

bool rectangle_aatriangle(Constraints* constraints, const Rectf& rect,
                          const AATriangle& triangle,
                          bool& hits_rectangle_bottom);

void set_rectangle_rectangle_constraints(Constraints* constraints, const Rectf& r1, const Rectf& r2);

bool line_intersects_line(const Vector& line1_start, const Vector& line1_end, const Vector& line2_start, const Vector& line2_end);
bool intersects_line(const Rectf& r, const Vector& line_start, const Vector& line_end);

} // namespace collision

#endif

/* EOF */
