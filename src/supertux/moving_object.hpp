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

#ifndef HEADER_SUPERTUX_SUPERTUX_MOVING_OBJECT_HPP
#define HEADER_SUPERTUX_SUPERTUX_MOVING_OBJECT_HPP

#include <stdint.h>

#include "math/rect.hpp"
#include "supertux/collision_hit.hpp"
#include "supertux/game_object.hpp"

class Sector;
class CollisionGrid;

enum CollisionGroup {
  /** Objects in DISABLED group are not tested for collisions */
  COLGROUP_DISABLED = 0,

  /** Tested against:
      - tiles + attributes
      - static obstacles
      - touchables
      - other moving objects
      and it counts as an obstacle during static collision phase.

      Use for kinematic moving objects like platforms and rocks. */
  COLGROUP_MOVING_STATIC,

  /** Tested against:
      - tiles + attributes
      - static obstacles
      - touchables
      - other moving objects

      Use for ordinary objects. */
  COLGROUP_MOVING,

  /** Tested against:
      - tiles + attributes
      - static obstacles

      Use for interactive particles and decoration. */
  COLGROUP_MOVING_ONLY_STATIC,

  /** Tested against:
      - moving objects
      and it counts as an obstacle during static collision phase.

      Use for static obstacles that Tux walks on. */
  COLGROUP_STATIC,

  /** Tested against:
      - moving objects

      Use for triggers like spikes/areas or collectibles like coins. */
  COLGROUP_TOUCHABLE
};

/** Base class for all dynamic/moving game objects. This class
    contains things for handling the bounding boxes and collision
    feedback. */
class MovingObject : public GameObject
{
public:
  MovingObject();
  virtual ~MovingObject();

  /** this function is called when the object collided with something solid */
  virtual void collision_solid(const CollisionHit& hit)
  {
    (void) hit;
  }

  /** when 2 objects collided, we will first call the
      pre_collision_check functions of both objects that can decide on
      how to react to the collision. */
  virtual bool collides(GameObject& other, const CollisionHit& hit)
  {
    (void) other;
    (void) hit;
    return true;
  }

  /** this function is called when the object collided with any other object */
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) = 0;

  /** called when tiles with special attributes have been touched */
  virtual void collision_tile(uint32_t tile_attributes)
  {
    (void) tile_attributes;
  }

  const Vector& get_pos() const
  {
    return bbox.p1;
  }

  /** returns the bounding box of the Object */
  const Rect& get_bbox() const
  {
    return bbox;
  }

  const Vector& get_movement() const
  {
    return movement;
  }

  /** places the moving object at a specific position. Be careful when
      using this function. There are no collision detection checks
      performed here so bad things could happen. */
  virtual void set_pos(const Vector& pos)
  {
    dest.move(pos-get_pos());
    bbox.set_pos(pos);
  }

  /** sets the moving object's bbox to a specific width. Be careful
      when using this function. There are no collision detection
      checks performed here so bad things could happen. */
  virtual void set_width(float w)
  {
    dest.set_width(w);
    bbox.set_width(w);
  }

  /** sets the moving object's bbox to a specific size. Be careful
      when using this function. There are no collision detection
      checks performed here so bad things could happen. */
  virtual void set_size(float w, float h)
  {
    dest.set_size(w, h);
    bbox.set_size(w, h);
  }

  CollisionGroup get_group() const
  {
    return group;
  }

protected:
  friend class Sector;
  friend class CollisionGrid;
  friend class Platform;

  void set_group(CollisionGroup group)
  {
    this->group = group;
  }

  /** The bounding box of the object (as used for collision detection,
      this isn't necessarily the bounding box for graphics) */
  Rect bbox;

  /** The movement that will happen till next frame */
  Vector movement;

  /** The collision group */
  CollisionGroup group;

private:
  /** this is only here for internal collision detection use (don't touch this
      from outside collision detection code)
      
      This field holds the currently anticipated destination of the object
      during collision detection */
  Rect dest;
};

#endif

/* EOF */
