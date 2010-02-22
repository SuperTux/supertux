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

#ifndef HEADER_SUPERTUX_SUPERTUX_COLLISION_HPP
#define HEADER_SUPERTUX_SUPERTUX_COLLISION_HPP

#include "supertux/collision_hit.hpp"
#include <limits>
#include <algorithm> /* min/max */

class Vector;
class Rectf;
class AATriangle;

namespace collision {

class Constraints
{
public:
  Constraints() :
    position_left(),
    position_right(),
    position_top(),
    position_bottom(),
    speed_left(),
    speed_right(),
    speed_top(),
    speed_bottom(),
    ground_movement(),
    hit()
  {
    float infinity = (std::numeric_limits<float>::has_infinity ? 
                      std::numeric_limits<float>::infinity() : 
                      std::numeric_limits<float>::max());
    position_left = -infinity;
    position_right = infinity;
    position_top = -infinity;
    position_bottom = infinity;

    speed_left = -infinity;
    speed_right = infinity;
    speed_top = -infinity;
    speed_bottom = infinity;
  }

  bool has_constraints() const 
  {
    float infinity = (std::numeric_limits<float>::has_infinity ?
                      std::numeric_limits<float>::infinity() : 
                      std::numeric_limits<float>::max());
    return
      position_left   > -infinity || 
      position_right  <  infinity || 
      position_top    > -infinity || 
      position_bottom <  infinity;
  }

public:

  void constrain_left (float position, float velocity)
  {
    position_left = std::max (position_left, position);
    speed_left = std::max (speed_left, velocity);
  }

  void constrain_right (float position, float velocity)
  {
    position_right = std::min (position_right, position);
    speed_right = std::min (speed_right, velocity);
  }

  void constrain_top (float position, float velocity)
  {
    position_top = std::max (position_top, position);
    speed_top = std::max (speed_top, velocity);
  }

  void constrain_bottom (float position, float velocity)
  {
    position_bottom = std::min (position_bottom, position);
    speed_bottom = std::min (speed_bottom, velocity);
  }

  float get_position_left   (void) const { return position_left;   }
  float get_position_right  (void) const { return position_right;  }
  float get_position_top    (void) const { return position_top;    }
  float get_position_bottom (void) const { return position_bottom; }

  float get_height (void) const { return (position_bottom - position_top); }
  float get_width  (void) const { return (position_right - position_left); }

  float get_x_midpoint (void) const { return (.5f * (position_left + position_right)); }

  Vector ground_movement;
  CollisionHit hit;

private:
  float position_left;
  float position_right;
  float position_top;
  float position_bottom;

  float speed_left;
  float speed_right;
  float speed_top;
  float speed_bottom;
};

/** checks if 2 rectangle intersect each other */
bool intersects(const Rectf& r1, const Rectf& r2);

/** does collision detection between a rectangle and an axis aligned triangle
 * Returns true in case of a collision and fills in the hit structure then.
 */
bool rectangle_aatriangle(Constraints* constraints, const Rectf& rect,
                          const AATriangle& triangle, const Vector& addl_ground_movement = Vector(0,0));

void set_rectangle_rectangle_constraints(Constraints* constraints,
                                         const Rectf& r1, const Rectf& r2, const Vector& addl_ground_movement = Vector(0,0));

} // namespace collision

#endif

/* EOF */
