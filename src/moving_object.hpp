//  $Id: moving_object.h 2295 2005-03-30 01:52:14Z matzebraun $
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
#ifndef SUPERTUX_MOVING_OBJECT_H
#define SUPERTUX_MOVING_OBJECT_H

#include <stdint.h>

#include "game_object.hpp"
#include "collision_hit.hpp"
#include "math/vector.hpp"
#include "math/rect.hpp"

class Sector;
class CollisionGrid;

enum CollisionGroup {
  COLGROUP_DISABLED,
  COLGROUP_MOVING,
  // moving object but don't collide against other moving objects
  COLGROUP_MOVING_ONLY_STATIC,
  COLGROUP_STATIC,
  COLGROUP_MOVINGSTATIC,
  COLGROUP_TOUCHABLE,
  
  COLGROUP_TILEMAP /* not really used at the moment */
};

/**
 * Base class for all dynamic/moving game objects. This class contains things
 * for handling the bounding boxes and collision feedback.
 */
class MovingObject : public GameObject
{
public:
  MovingObject();
  virtual ~MovingObject();
  
  /** this function is called when the object collided with any other object
   */
  virtual HitResponse collision(GameObject& other,
                                const CollisionHit& hit) = 0;
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
  
  /** places the moving object at a specific position. Be carefull when
   * using this function. There are no collision detection checks performed
   * here so bad things could happen.
   */
  virtual void set_pos(const Vector& pos)
  {
    bbox.set_pos(pos);
  }

  CollisionGroup get_group() const
  {
    return group;
  }

  void set_group(CollisionGroup group)
  {
    this->group = group;
  }
  
protected:
  friend class Sector;
  friend class CollisionGrid;
  friend class Platform;
  
  /** The bounding box of the object (as used for collision detection, this
   * isn't necessarily the bounding box for graphics)
   */
  Rect bbox;
  /** The movement that will happen till next frame
   */
  Vector movement;
  /** The collision group */
  CollisionGroup group;

private:
  // this is only here for internal collision detection use
  Rect dest;
};

#endif
