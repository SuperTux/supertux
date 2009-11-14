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
#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <float.h>
#include "collision_hit.hpp"
#include <limits>

class Vector;
class Rect;
class AATriangle;

namespace collision
{

class Constraints
{
public:
  Constraints() {
    float infinity = (std::numeric_limits<float>::has_infinity ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max());
    left = -infinity;
    right = infinity;
    top = -infinity;
    bottom = infinity;
  }

  bool has_constraints() const {
    float infinity = (std::numeric_limits<float>::has_infinity ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max());
    return left > -infinity || right < infinity
        || top > -infinity || bottom < infinity;
  }

  float left;
  float right;
  float top;
  float bottom;
  Vector ground_movement;
  CollisionHit hit;
};

/** checks if 2 rectangle intersect each other */
bool intersects(const Rect& r1, const Rect& r2);

/** does collision detection between a rectangle and an axis aligned triangle
 * Returns true in case of a collision and fills in the hit structure then.
 */
bool rectangle_aatriangle(Constraints* constraints, const Rect& rect,
                                   const AATriangle& triangle, const Vector& addl_ground_movement = Vector(0,0));

void set_rectangle_rectangle_constraints(Constraints* constraints,
        const Rect& r1, const Rect& r2, const Vector& addl_ground_movement = Vector(0,0));

}

#endif
