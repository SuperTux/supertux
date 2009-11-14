//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef SUPERTUX_COLLISION_HIT_H
#define SUPERTUX_COLLISION_HIT_H

#include "math/vector.hpp"

/**
 * Used as return value for the collision functions, to indicate how the
 * collision should be handled
 */
enum HitResponse
{
  /// don't move the object
  ABORT_MOVE = 0,
  /// move object out of collision and check for collisions again
  /// if this happens to often then the move will just be aborted
  CONTINUE,
  /// do the move ignoring the collision
  FORCE_MOVE,
  /// passes movement to collided object
  PASS_MOVEMENT,

  /// the object should not appear solid
  PASSTHROUGH,
  /// the object should appear solid
  SOLID,
};

/**
 * This class collects data about a collision
 */
class CollisionHit
{
public:
  CollisionHit() {
    left = false;
    right = false;
    top = false;
    bottom = false;
    crush = false;
  }

  bool left, right;
  bool top, bottom;
  bool crush;

  Vector slope_normal;
};

#endif
