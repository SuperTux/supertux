//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2018 Bobby Youstra <bobbyyoustra@protonmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_SECTOR_COLLISION_DETECTOR_HPP
#define HEADER_SUPERTUX_OBJECT_SECTOR_COLLISION_DETECTOR_HPP

#include <list>
#include <vector>

#include "math/aatriangle.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "supertux/collision.hpp"
#include "supertux/constants.hpp"

class SectorCollisionDetector {
friend class Sector; // to allow Sector to use handle_collisions()
public:
  SectorCollisionDetector(std::vector<MovingObject*>& moving_objects,
                          std::list<TileMap*>& solid_tilemaps,
                          std::vector<Player*> players);
                          
  void set_players(std::vector<Player*> players);
  
  void collision_tilemap(collision::Constraints* constraints,
                         const Vector& movement, const Rectf& dest,
                         MovingObject& object) const;
                          
   /**
   * Checks if the specified rectangle is free of (solid) tiles.
   * Note that this does not include static objects, e.g. bonus blocks.
   */
  bool is_free_of_tiles(const Rectf& rect, 
                        const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC.
   * Note that this does not include badguys or players.
   */
  bool is_free_of_statics(const Rectf& rect, 
                          const MovingObject* ignore_object = 0, 
                          const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC, COLGROUP_MOVINGSTATIC or COLGROUP_MOVING.
   * This includes badguys and players.
   */
  bool is_free_of_movingstatics(const Rectf& rect, 
                                const MovingObject* ignore_object = 0) const;
                                
  bool free_line_of_sight(const Vector& line_start, const Vector& line_end, 
                          const MovingObject* ignore_object = 0) const;
  bool can_see_player(const Vector& eye) const;

private:                          
  uint32_t collision_tile_attributes(const Rectf& dest, 
                                     const Vector& mov) const;
                                     
  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void handle_collisions();

  /**
   * Does collision detection between 2 objects and does instant
   * collision response handling in case of a collision
   */
  void collision_object(MovingObject* object1, MovingObject* object2) const;

  /**
   * Does collision detection of an object against all other static
   * objects (and the tilemap) in the level. Collision response is done
   * for the first hit in time. (other hits get ignored, the function
   * should be called repeatedly to resolve those)
   *
   * returns true if the collision detection should be aborted for this object
   * (because of ABORT_MOVE in the collision response or no collisions)
   */
  void collision_static(collision::Constraints* constraints,
                        const Vector& movement, const Rectf& dest, MovingObject& object);

  void collision_static_constrains(MovingObject& object);
  
  std::vector<MovingObject*>& m_moving_objects;
  
  std::list<TileMap*>& m_solid_tilemaps;
  std::vector<Player*> m_players;
};

#endif

/* EOF */
