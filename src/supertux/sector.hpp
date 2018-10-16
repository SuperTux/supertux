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

#include "object/anchor_point.hpp"
#include "supertux/game_object_ptr.hpp"
#include "supertux/game_object_manager.hpp"
#include "supertux/script_engine.hpp"
#include "video/color.hpp"

namespace collision {
class Constraints;
}

class Bullet;
class Camera;
class CollisionSystem;
class DisplayEffect;
class DrawingContext;
class Level;
class MovingObject;
class Player;
class Portable;
class ReaderMapping;
class Rectf;
class Size;
class SpawnPoint;
class TileMap;
class Vector;
class Writer;

enum MusicType {
  LEVEL_MUSIC,
  HERRING_MUSIC,
  HERRING_WARNING_MUSIC
};

/**
 * Represents one of (potentially) multiple, separate parts of a Level.
 *
 * Sectors contain GameObjects, e.g. Badguys and Players.
 */
class Sector final : public GameObjectManager,
                     public ScriptEngine
{
public:
  friend class EditorSectorMenu;
  friend class CollisionSystem;

public:
  Sector(Level& parent);
  ~Sector();

  /** Needs to be called after parsing to finish the construction of
      the Sector before using it. */
  void construct();

  Level& get_level() const;

  /// activates this sector (change music, initialize player class, ...)
  void activate(const std::string& spawnpoint);
  void activate(const Vector& player_pos);
  void deactivate();

  void update(float elapsed_time);

  void draw(DrawingContext& context);

  void save(Writer &writer);

  /// stops all looping sounds in whole sector.
  void stop_looping_sounds();

  /// continues the looping sounds in whole sector.
  void play_looping_sounds();

  void set_name(const std::string& name_)
  { m_name = name_; }
  const std::string& get_name() const
  { return m_name; }

  /**
   * tests if a given rectangle is inside the sector
   * (a rectangle that is on top of the sector is considered inside)
   */
  bool inside(const Rectf& rectangle) const;

  void play_music(MusicType musictype);
  void resume_music();
  MusicType get_music_type() const;

  int get_active_bullets() const
  { return static_cast<int>(m_bullets.size()); }

  /** get currently activated sector. */
  static Sector* current()
  { return s_current; }

  /** Get total number of badguys */
  int get_total_badguys() const;

  /**
   * Checks if the specified rectangle is free of (solid) tiles.
   * Note that this does not include static objects, e.g. bonus blocks.
   */
  bool is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC.
   * Note that this does not include badguys or players.
   */
  bool is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object = nullptr, const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC, COLGROUP_MOVINGSTATIC or COLGROUP_MOVING.
   * This includes badguys and players.
   */
  bool is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object = nullptr) const;

  bool free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object = nullptr) const;
  bool can_see_player(const Vector& eye) const;

/**
   * returns a list of players currently in the sector
   */
  std::vector<Player*> get_players() const {
    return std::vector<Player*>(1, m_player);
  }
  Player* get_nearest_player (const Vector& pos) const;
  Player* get_nearest_player (const Rectf& pos) const
  {
    return (get_nearest_player (get_anchor_pos (pos, ANCHOR_MIDDLE)));
  }

  std::vector<MovingObject*> get_nearby_objects (const Vector& center, float max_distance) const;

  Rectf get_active_region() const;

  int get_foremost_layer() const;

  /**
   * returns the width (in px) of a sector)
   */
  float get_width() const;

  /**
   * returns the height (in px) of a sector)
   */
  float get_height() const;

  /**
   * returns the editor size (in tiles) of a sector
   */
  Size get_editor_size() const;

  /**
   * resize all tilemaps with given size
   */
  void resize_sector(const Size& old_size, const Size& new_size, const Size& resize_offset);

  /**
   * globally changes solid tilemaps' tile ids
   */
  void change_solid_tiles(uint32_t old_tile_id, uint32_t new_tile_id);

  // --- scripting ---
  /**
   *  get/set color of ambient light
   */
  void set_ambient_light(const Color& ambient_light);
  float get_ambient_red() const;
  float get_ambient_green() const;
  float get_ambient_blue() const;

  /**
   * Return the sector's current ambient light
   */
  Color get_ambient_light() const
  {
    return m_ambient_light;
  }

  /**
   * Fades to the target ambient light
   */
  void fade_to_ambient_light(float red, float green, float blue, float seconds);

  /**
   *  set gravity throughout sector
   */
  void set_gravity(float gravity);
  float get_gravity() const;

  void set_music(const std::string& music);
  std::string get_music() const;

  void set_init_script(const std::string& init_script) {
    m_init_script = init_script;
  }

  const std::vector<MovingObject*>& get_moving_objects() const;

private:
  uint32_t collision_tile_attributes(const Rectf& dest, const Vector& mov) const;

  virtual bool before_object_add(GameObjectPtr object) override;
  virtual void before_object_remove(GameObjectPtr object) override;

  int calculate_foremost_layer() const;

  /** Convert tiles into their corresponding GameObjects (e.g.
      bonusblocks, add light to lava tiles) */
  void convert_tiles2gameobject();

private:
  static Sector* s_current;

public: // TODO make this private again
  /// show collision rectangles of moving objects (for debugging)
  static bool s_show_collrects;
  static bool s_draw_solids_only;

private:
  Level& m_level; /**< Parent level containing this sector */

  std::string m_name;

  std::vector<Bullet*> m_bullets;

  std::string m_init_script;

  MusicType m_currentmusic;

  Color m_ambient_light;

  /**
   * Specifies whether we're fading the ambient light
   */
  bool m_ambient_light_fading;

  /**
   * Source color for fading
   */
  Color m_source_ambient_light;

  /**
   * Target color for fading
   */
  Color m_target_ambient_light;

  /**
   * Ambient light fade duration
   */
   float m_ambient_light_fade_duration;

  /**
   * Accumulated time for fading
   */
   float m_ambient_light_fade_accum;

  int m_foremost_layer;

private:
  std::unique_ptr<CollisionSystem> m_collision_system;

  float m_gravity;
  std::string m_music;

public:
  // some special objects, where we need direct access
  // (try to avoid accessing them directly)
  std::vector<std::shared_ptr<SpawnPoint> > m_spawnpoints;
  std::vector<Portable*> m_portables;
  Player* m_player;
  std::vector<TileMap*> m_solid_tilemaps;
  Camera* m_camera;
  DisplayEffect* m_effect;

private:
  Sector(const Sector&);
  Sector& operator=(const Sector&);
};

#endif

/* EOF */
