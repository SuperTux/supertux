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

#include "math/rectf.hpp"
#include "supertux/collision_hit.hpp"
#include "supertux/collision_object.hpp"
#include "supertux/game_object.hpp"

class Sector;

/** Base class for all dynamic/moving game objects. This class
    contains things for handling the bounding boxes and collision
    feedback. */
class MovingObject : public GameObject
{
  friend class Sector;
  friend class CollisionSystem;

public:
  MovingObject();
  MovingObject(const ReaderMapping& reader);
  virtual ~MovingObject();

  /** this function is called when the object collided with something solid */
  virtual void collision_solid(const CollisionHit& /*hit*/)
  {
  }

  /** when 2 objects collided, we will first call the
      pre_collision_check functions of both objects that can decide on
      how to react to the collision. */
  virtual bool collides(GameObject& /*other*/, const CollisionHit& /*hit*/) const
  {
    return true;
  }

  /** this function is called when the object collided with any other object */
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) = 0;

  /** called when tiles with special attributes have been touched */
  virtual void collision_tile(uint32_t /*tile_attributes*/)
  {
  }

  virtual void set_pos(const Vector& pos)
  {
    m_col.set_pos(pos);
  }

  virtual void move_to(const Vector& pos)
  {
    m_col.move_to(pos);
  }

  const Vector& get_pos() const
  {
    return m_col.m_bbox.p1;
  }

  const Rectf& get_bbox() const
  {
    return m_col.m_bbox;
  }

  const Vector& get_movement() const
  {
    return m_col.m_movement;
  }

  CollisionGroup get_group() const
  {
    return m_col.m_group;
  }

  /** This function saves the object.
   *  Editor will use that.
   */
  virtual void save(Writer& writer) override;
  virtual std::string get_class() const override {
    return "moving-object";
  }

  /** puts resizers at its edges, used in editor input center */
  void edit_bbox();

protected:
  void set_group(CollisionGroup group)
  {
    m_col.m_group = group;
  }

protected:
  CollisionObject m_col;
};

#endif

/* EOF */
