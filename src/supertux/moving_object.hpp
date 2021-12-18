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

#include "collision/collision_hit.hpp"
#include "collision/collision_object.hpp"
#include "collision/collision_listener.hpp"
#include "math/rectf.hpp"
#include "supertux/game_object.hpp"

class Sector;

/** Base class for all dynamic/moving game objects. This class
    contains things for handling the bounding boxes and collision
    feedback. */
class MovingObject : public GameObject,
                     public CollisionListener
{
  friend class Sector;
  friend class CollisionSystem;

public:
  MovingObject();
  MovingObject(const ReaderMapping& reader);
  ~MovingObject() override;

  virtual void collision_solid(const CollisionHit& /*hit*/) override
  {
  }

  virtual bool collides(GameObject& /*other*/, const CollisionHit& /*hit*/) const override
  {
    return true;
  }

  virtual void collision_tile(uint32_t /*tile_attributes*/) override
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

  virtual bool listener_is_valid() const override { return is_valid(); }

  Vector get_pos() const
  {
    return m_col.m_bbox.p1();
  }

  const Rectf& get_bbox() const
  {
    return m_col.m_bbox;
  }

  const Vector& get_movement() const
  {
    return m_col.get_movement();
  }

  CollisionGroup get_group() const
  {
    return m_col.m_group;
  }

  CollisionObject* get_collision_object() {
    return &m_col;
  }

  const CollisionObject* get_collision_object() const {
    return &m_col;
  }

  virtual std::string get_class() const override { return "moving-object"; }
  virtual ObjectSettings get_settings() override;

  virtual void editor_select() override;

  virtual void on_flip(float height) override;

  virtual int get_layer() const = 0;

protected:
  void set_group(CollisionGroup group)
  {
    m_col.m_group = group;
  }

protected:
  CollisionObject m_col;

private:
  MovingObject(const MovingObject&) = delete;
  MovingObject& operator=(const MovingObject&) = delete;
};

#endif

/* EOF */
