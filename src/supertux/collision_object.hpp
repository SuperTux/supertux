//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_COLLISION_OBJECT_HPP
#define HEADER_SUPERTUX_SUPERTUX_COLLISION_OBJECT_HPP

#include <stdint.h>

#include "math/rectf.hpp"
#include "supertux/collision_hit.hpp"

class GameObject;
class MovingObject;

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

class CollisionObject
{
  friend class CollisionSystem;

public:
  CollisionObject(CollisionGroup group, MovingObject& parent);

  /** this function is called when the object collided with something solid */
  void collision_solid(const CollisionHit& hit);

  /** when 2 objects collided, we will first call the
      pre_collision_check functions of both objects that can decide on
      how to react to the collision. */
  bool collides(GameObject& other, const CollisionHit& hit) const;

  /** this function is called when the object collided with any other object */
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  /** called when tiles with special attributes have been touched */
  void collision_tile(uint32_t tile_attributes);

  /** returns the bounding box of the Object */
  const Rectf& get_bbox() const
  {
    return m_bbox;
  }

  const Vector& get_movement() const
  {
    return m_movement;
  }

  /** places the moving object at a specific position. Be careful when
      using this function. There are no collision detection checks
      performed here so bad things could happen. */
  void set_pos(const Vector& pos)
  {
    m_dest.move(pos - get_pos());
    m_bbox.set_pos(pos);
  }

  const Vector& get_pos() const
  {
    return m_bbox.p1;
  }

  /** moves entire object to a specific position, including all
      points those the object has, exactly like the object has
      spawned in that given pos instead.*/
  void move_to(const Vector& pos)
  {
    set_pos(pos);
  }

  /** sets the moving object's bbox to a specific width. Be careful
      when using this function. There are no collision detection
      checks performed here so bad things could happen. */
  void set_width(float w)
  {
    m_dest.set_width(w);
    m_bbox.set_width(w);
  }

  /** sets the moving object's bbox to a specific size. Be careful
      when using this function. There are no collision detection
      checks performed here so bad things could happen. */
  void set_size(float w, float h)
  {
    m_dest.set_size(w, h);
    m_bbox.set_size(w, h);
  }

  CollisionGroup get_group() const
  {
    return m_group;
  }

private:
  MovingObject& m_parent;

public:
  /** The bounding box of the object (as used for collision detection,
      this isn't necessarily the bounding box for graphics) */
  Rectf m_bbox;

  /** The movement that will happen till next frame */
  Vector m_movement;

  /** The collision group */
  CollisionGroup m_group;

private:
  /** this is only here for internal collision detection use (don't touch this
      from outside collision detection code)

      This field holds the currently anticipated destination of the object
      during collision detection */
  Rectf m_dest;

private:
  CollisionObject(const CollisionObject&) = delete;
  CollisionObject& operator=(const CollisionObject&) = delete;
};

#endif

/* EOF */
