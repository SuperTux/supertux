//  SuperTux -  A Jump'n Run
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SECTOR_HPP
#define HEADER_SUPERTUX_SUPERTUX_SECTOR_HPP

#include <vector>
#include <stdint.h>

#include "math/anchor_point.hpp"
#include "math/easing.hpp"
#include "squirrel/squirrel_environment.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/game_object_manager.hpp"
#include "supertux/tile.hpp"
#include "video/color.hpp"

namespace collision {
class Constraints;
}

class Camera;
class CollisionSystem;
class CollisionGroundMovementManager;
class DisplayEffect;
class DrawingContext;
class Level;
class MovingObject;
class Player;
class ReaderMapping;
class Rectf;
class Size;
class TileMap;
class Vector;
class Writer;

/** Represents one of (potentially) multiple, separate parts of a Level.
    Sectors contain GameObjects, e.g. Badguys and Players. */
class Sector final : public GameObjectManager
{
public:
  friend class CollisionSystem;
  friend class EditorSectorMenu;

private:
  static Sector* s_current;

public:
  /** get currently activated sector. */
  static Sector& get() { assert(s_current != nullptr); return *s_current; }
  static Sector* current() { return s_current; }

public:
  Sector(Level& parent);
  ~Sector() override;

  /** Needs to be called after parsing to finish the construction of
      the Sector before using it. */
  void finish_construction(bool editable);

  Level& get_level() const;

  /** activates this sector (change music, initialize player class, ...) */
  void activate(const std::string& spawnpoint);
  void activate(const Vector& player_pos);
  void deactivate();

  void update(float dt_sec);

  void draw(DrawingContext& context);

  void save(Writer &writer);

  /** stops all looping sounds in whole sector. */
  void stop_looping_sounds();

  /** continues the looping sounds in whole sector. */
  void play_looping_sounds();

  void set_name(const std::string& name_) { m_name = name_; }
  const std::string& get_name() const { return m_name; }

  /** tests if a given rectangle is inside the sector
      (a rectangle that is on top of the sector is considered inside) */
  bool inside(const Rectf& rectangle) const;

  /** Checks if the specified rectangle is free of (solid) tiles.
      Note that this does not include static objects, e.g. bonus blocks. */
  bool is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid = false, uint32_t tiletype = Tile::SOLID) const;

  /** Checks if the specified rectangle is free of both
      1.) solid tiles and
      2.) MovingObjects in COLGROUP_STATIC.
      Note that this does not include badguys or players. */
  bool is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object = nullptr, const bool ignoreUnisolid = false) const;

  /** Checks if the specified rectangle is free of both
      1.) solid tiles and
      2.) MovingObjects in COLGROUP_STATIC, COLGROUP_MOVINGSTATIC or COLGROUP_MOVING.
      This includes badguys and players. */
  bool is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object = nullptr) const;

  bool free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object = nullptr) const;
  bool can_see_player(const Vector& eye) const;

  Player* get_nearest_player (const Vector& pos) const;
  Player* get_nearest_player (const Rectf& pos) const {
    return (get_nearest_player (get_anchor_pos (pos, ANCHOR_MIDDLE)));
  }

  std::vector<MovingObject*> get_nearby_objects (const Vector& center, float max_distance) const;

  Rectf get_active_region() const;

  int get_foremost_layer() const;

  /** returns the editor size (in tiles) of a sector */
  Size get_editor_size() const;

  /** resize all tilemaps with given size */
  void resize_sector(const Size& old_size, const Size& new_size, const Size& resize_offset);

  /** globally changes solid tilemaps' tile ids */
  void change_solid_tiles(uint32_t old_tile_id, uint32_t new_tile_id);

  /** set gravity throughout sector */
  void set_gravity(float gravity);
  float get_gravity() const;

  void set_init_script(const std::string& init_script) {
    m_init_script = init_script;
  }

  void run_script(const std::string& script, const std::string& sourcename);

  Camera& get_camera() const;
  Player& get_player() const;
  DisplayEffect& get_effect() const;

private:
  uint32_t collision_tile_attributes(const Rectf& dest, const Vector& mov) const;

  virtual bool before_object_add(GameObject& object) override;
  virtual void before_object_remove(GameObject& object) override;

  int calculate_foremost_layer() const;

  /** Convert tiles into their corresponding GameObjects (e.g.
      bonusblocks, add light to lava tiles) */
  void convert_tiles2gameobject();

private:
  /** Parent level containing this sector */
  Level& m_level;

  std::string m_name;

  bool m_fully_constructed;

  std::string m_init_script;

  int m_foremost_layer;

  std::unique_ptr<SquirrelEnvironment> m_squirrel_environment;
  std::unique_ptr<CollisionSystem> m_collision_system;

  float m_gravity;

private:
  Sector(const Sector&) = delete;
  Sector& operator=(const Sector&) = delete;
};

#endif

/* EOF */
