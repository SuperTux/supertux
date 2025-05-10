//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_COLLISION_COLLISION_SYSTEM_HPP
#define HEADER_SUPERTUX_COLLISION_COLLISION_SYSTEM_HPP

#include <vector>
#include <memory>
#include <variant>
#include <stdint.h>

#include "collision/collision.hpp"
#include "supertux/tile.hpp"
#include "math/fwd.hpp"

class MovingObject;
class CollisionGroundMovementManager;
class DrawingContext;
class Rectf;
class Sector;
class MovingObject;

class CollisionSystem final
{
public:
  struct RaycastResult
  {
    bool is_valid = false; /**< true if raycast hit something */
    std::variant<const Tile*, MovingObject*> hit; /**< tile/object that the raycast hit */
    Rectf box = {}; /**< hitbox of tile/object */
  };

public:
  CollisionSystem(Sector& sector);

  void add(MovingObject* object);
  void remove(MovingObject* object);

  /** Draw collision shapes for debugging */
  void draw(DrawingContext& context);

  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void update();

  const std::shared_ptr<CollisionGroundMovementManager>& get_ground_movement_manager()
  {
    return m_ground_movement_manager;
  }

  bool is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid = false, uint32_t tiletype = Tile::SOLID) const;
  bool is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object, const bool ignoreUnisolid, uint32_t tiletype = Tile::SOLID) const;
  bool is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const;
  bool is_free_of_specifically_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const;

  enum RaycastIgnore {
    IGNORE_NONE,
    IGNORE_TILES,
    IGNORE_OBJECTS
  };

  RaycastResult get_first_line_intersection(const Vector& line_start,
                                            const Vector& line_end,
                                            RaycastIgnore ignore = IGNORE_NONE,
                                            const MovingObject* ignore_object = nullptr) const;
  bool free_line_of_sight(const Vector& line_start, const Vector& line_end, bool ignore_objects, const MovingObject* ignore_object) const;

  std::vector<MovingObject*> get_nearby_objects(const Vector& center, float max_distance) const;

private:
  /** Does collision detection of an object against all other static
      objects (and the tilemap) in the level. Collision response is
      done for the first hit in time. (other hits get ignored, the
      function should be called repeatedly to resolve those)

      returns true if the collision detection should be aborted for
      this object (because of ABORT_MOVE in the collision response or
      no collisions) */
  void collision_static(collision::Constraints* constraints,
                        const Vector& movement, const Rectf& dest,
                        MovingObject& object);

  void collision_tilemap(collision::Constraints* constraints,
                         const Vector& movement, const Rectf& dest,
                         MovingObject& object) const;

  uint32_t collision_tile_attributes(const Rectf& dest, const Vector& mov) const;

  void collision_object(MovingObject* object1, MovingObject* object2) const;

  void collision_static_constrains(MovingObject& object);

  void get_hit_normal(const MovingObject* object1, const MovingObject* object2,
                      CollisionHit& hit, Vector& normal) const;

private:
  Sector& m_sector;

  std::vector<MovingObject*>  m_objects;

  std::shared_ptr<CollisionGroundMovementManager> m_ground_movement_manager;

private:
  CollisionSystem(const CollisionSystem&) = delete;
  CollisionSystem& operator=(const CollisionSystem&) = delete;
};

#endif

/* EOF */
