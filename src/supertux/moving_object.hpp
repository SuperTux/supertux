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
#include <memory>
#include <unordered_set>
#include "supertux/game_object.hpp"

#include "collision/collision_hit.hpp"
#include "collision/collision_object.hpp"
#include "collision/collision_group.hpp"
#include "collision/collision_movement_manager.hpp"
#include "math/rectf.hpp"

class Dispenser;
class Sector;

/**
 * @scripting
 * @summary Base class for all dynamic/moving game objects. This class
            contains things for handling the bounding boxes and collision
            feedback.
 */
class MovingObject : public GameObject
{
  friend class ResizeMarker;
  friend class Sector;
  friend class CollisionSystem;

// Properties that were moved from CollisionObject
  
public:
  /** The bounding box of the object (as used for collision detection,
      this isn't necessarily the bounding box for graphics) */
  Rectf m_bbox;

  /** The collision group */
  CollisionGroup m_group;

private:
  /** The movement that will happen till next frame */
  Vector m_movement;

  /** This is only here for internal collision detection use (don't touch this
      from outside collision detection code)

      This field holds the currently anticipated destination of the object
      during collision detection */
  Rectf m_dest;

  /** Determines whether the object is unisolid.

      Only bottom constraints to the top of the bounding box would be applied for objects,
      colliding with unisolid objects. */
  bool m_unisolid;

  /**
   * Contains the current pressure on this object
   */
  Vector m_pressure;

  /** Objects that were touching the top of this object at the last frame,
      if this object was static or moving static. */
  std::unordered_set<MovingObject*> m_objects_hit_bottom;

  std::shared_ptr<CollisionGroundMovementManager> m_ground_movement_manager;

// END Properties that were moved from CollisionObject

public:
  static void register_class(ssq::VM& vm);

public:
  MovingObject();
  MovingObject(const ReaderMapping& reader);
  ~MovingObject() override;
  virtual GameObjectClasses get_class_types() const override { return GameObject::get_class_types().add(typeid(MovingObject)); }

  virtual void collision_solid(const CollisionHit& /*hit*/)
  {
  }

  virtual bool collides(MovingObject& /*other*/, const CollisionHit& /*hit*/) const
  {
    return true;
  }

  virtual HitResponse collision(MovingObject& other, const CollisionHit& hit) = 0;

  virtual void collision_tile(uint32_t /*tile_attributes*/)
  {
  }

  inline Vector get_pressure() const
  {
    return m_pressure;
  }

  virtual void set_pos(const Vector& pos)
  {
    m_dest.move(pos - get_pos());
    m_bbox.set_pos(pos);
  }

  virtual void move_to(const Vector& pos)
  {
    set_pos(pos);
  }
  virtual void move(const Vector& dist)
  {
    m_bbox.move(dist);
  }

  Vector get_pos() const
  {
    return m_bbox.p1();
  }

  void set_size(float w, float h)
  {
    m_dest.set_size(w, h);
    m_bbox.set_size(w, h);
  }

  void set_width(float w)
  {
    m_dest.set_width(w);
    m_bbox.set_width(w);
  }

  /** called when this object, if (moving) static, has collided on its top with a moving object */
  void collision_moving_object_bottom(MovingObject& other);

  void notify_object_removal(MovingObject* other);

  inline void set_ground_movement_manager(const std::shared_ptr<CollisionGroundMovementManager>& movement_manager)
  {
    m_ground_movement_manager = movement_manager;
  }

  //Imported from CollisionObject
  void clear_bottom_collision_list();

  //Imported from CollisionObject
  inline bool is_unisolid() const { return m_unisolid; }
  inline void set_unisolid(bool unisolid) { m_unisolid = unisolid; }

  const Rectf& get_bbox() const
  {
    return m_bbox;
  }

  const Vector& get_movement() const
  {
    return m_movement;
  }

  //Imported from CollisionObject
  inline void set_movement(const Vector& movement)
  {
    m_movement = movement;
  }

  //Imported from CollisionObject
  void propagate_movement(const Vector& movement);

  CollisionGroup get_group() const
  {
    return m_group;
  }

  MovingObject* get_collision_object() {
    return this;
  }

  const MovingObject* get_collision_object() const {
    return this;
  }

  void set_parent_dispenser(Dispenser* dispenser);
  inline Dispenser* get_parent_dispenser() const { return m_parent_dispenser; }

  static std::string class_name() { return "moving-object"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "MovingObject"; }
  virtual ObjectSettings get_settings() override;

  virtual void editor_select() override;

  virtual void on_flip(float height) override;

  virtual int get_layer() const = 0;

  /**
   * @scripting
   * @description Returns the object's X coordinate.
   */
  inline float get_x() const { return m_bbox.get_left(); }
  /**
   * @scripting
   * @description Returns the object's Y coordinate.
   */
  inline float get_y() const { return m_bbox.get_top(); }
  /**
   * @scripting
   * @description Sets the position of the object.
   * @param float $x
   * @param float $y
   */
  inline void set_pos(float x, float y) { set_pos(Vector(x, y)); }
  /**
   * @scripting
   * @description Moves the object by ""x"" units to the right and ""y"" down, relative to its current position.
   * @param float $x
   * @param float $y
   */
  inline void move(float x, float y) { move(Vector(x, y)); }

  /**
   * @scripting
   * @description Returns the object's hitbox width.
   */
  inline float get_width() const { return m_bbox.get_width(); }
  /**
   * @scripting
   * @description Returns the object's hitbox height.
   */
  inline float get_height() const { return m_bbox.get_height(); }

protected:
  void set_group(CollisionGroup group)
  {
    m_group = group;
  }

protected:

  Dispenser* m_parent_dispenser;

private:
  MovingObject(const MovingObject&) = delete;
  MovingObject& operator=(const MovingObject&) = delete;
};

#endif

/* EOF */
