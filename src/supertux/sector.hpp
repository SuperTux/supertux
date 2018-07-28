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

#include <list>
#include <squirrel.h>
#include <stdint.h>

#include "object/anchor_point.hpp"
#include "supertux/direction.hpp"
#include "supertux/sector_collision_detector.hpp"
#include "supertux/game_object_ptr.hpp"
#include "video/color.hpp"

namespace collision {
class Constraints;
}

class Size;
class Vector;
class Rectf;
class Player;
class Camera;
class TileMap;
class Bullet;
class SpawnPoint;
class MovingObject;
class Level;
class Portable;
class DrawingContext;
class DisplayEffect;
class ReaderMapping;
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
class Sector
{
public:
  friend class SectorParser;
  friend class EditorSectorMenu;

public:
  Sector(Level* parent);
  ~Sector();

  /// get parent level
  Level* get_level() const;

  /// activates this sector (change music, initialize player class, ...)
  void activate(const std::string& spawnpoint);
  void activate(const Vector& player_pos);
  void deactivate();

  void update(float elapsed_time);
  void update_game_objects();

  void draw(DrawingContext& context);

  void on_window_resize();

  void save(Writer &writer);

  /// stops all looping sounds in whole sector.
  void stop_looping_sounds();

  /// continues the looping sounds in whole sector.
  void play_looping_sounds();

  /**
   * Convenience function that takes an std::string instead of an std::istream&
   */
  HSQUIRRELVM run_script(const std::string& script, const std::string& sourcename);

  /**
   * runs a script in the context of the sector (sector_table will be the
   * roottable of this squirrel VM)
   */
  HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename);

  /// adds a gameobject
  void add_object(GameObjectPtr object);

  void set_name(const std::string& name_)
  { name = name_; }
  const std::string& get_name() const
  { return name; }

  /**
   * tests if a given rectangle is inside the sector
   * (a rectangle that is on top of the sector is considered inside)
   */
  bool inside(const Rectf& rectangle) const;

  void play_music(MusicType musictype);
  void resume_music();
  MusicType get_music_type() const;

  int get_active_bullets() const
  { return (int)bullets.size(); }
  bool add_smoke_cloud(const Vector& pos);

  /** get currently activated sector. */
  static Sector* current()
  { return _current; }

  /** Get total number of badguys */
  int get_total_badguys() const;

  /** Get total number of GameObjects of given type */
  template<class T> int get_total_count() const
  {
    int total = 0;
    for(const auto& obj : gameobjects) {
      if (dynamic_cast<T*>(obj.get())) total++;
    }
    return total;
  }

  void collision_tilemap(collision::Constraints* constraints,
                         const Vector& movement, const Rectf& dest,
                         MovingObject &object) const;

  /**
   * Wrapper for SectorCollisionDetector's is_free_of_tiles function.
   */
  bool is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid = false) const;
  /**
   * Wrapper for SectorCollisionDetector's is_free_of_statics function.
   */
  bool is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object = 0, const bool ignoreUnisolid = false) const;
  /**
   * Wrapper for SectorCollisionDetector's is_free_of_movingstatics function.
   */
  bool is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object = 0) const;

  bool free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object = 0) const;
  bool can_see_player(const Vector& eye);

/**
   * returns a list of players currently in the sector
   */
  std::vector<Player*> get_players() const {
    return players;
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

  typedef std::vector<GameObjectPtr> GameObjects;
  typedef std::vector<MovingObject*> MovingObjects;
  typedef std::vector<std::shared_ptr<SpawnPoint> > SpawnPoints;
  typedef std::vector<Portable*> Portables;

  // --- scripting ---
  /**
   *  get/set color of ambient light
   */
  void set_ambient_light(float red, float green, float blue);
  float get_ambient_red() const;
  float get_ambient_green() const;
  float get_ambient_blue() const;

  /**
   * Return the sector's current ambient light
   */
  Color get_ambient_light() const
  {
    return ambient_light;
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

private:
  void before_object_remove(GameObjectPtr object);
  bool before_object_add(GameObjectPtr object);

  void try_expose(GameObjectPtr object);
  void try_unexpose(GameObjectPtr object);
  void try_expose_me();
  void try_unexpose_me();

  GameObjectPtr parse_object(const std::string& name, const ReaderMapping& lisp);

  void fix_old_tiles();

  int calculate_foremost_layer() const;

private:
  static Sector* _current;

  Level* level; /**< Parent level containing this sector */

  std::string name;

  std::vector<Bullet*> bullets;

  std::string init_script;

  /// container for newly created objects, they'll be added in Sector::update
  GameObjects gameobjects_new;

  MusicType currentmusic;

  HSQOBJECT sector_table;
  /// sector scripts
  typedef std::vector<HSQOBJECT> ScriptList;
  ScriptList scripts;

  Color ambient_light;

  /**
   * Specifies whether we're fading the ambient light
   */
  bool ambient_light_fading;

  /**
   * Source color for fading
   */
  Color source_ambient_light;

  /**
   * Target color for fading
   */
  Color target_ambient_light;

  /**
   * Ambient light fade duration
   */
   float ambient_light_fade_duration;

  /**
   * Accumulated time for fading
   */
   float ambient_light_fade_accum;

  int foremost_layer;

public: // TODO make this private again
  /// show collision rectangles of moving objects (for debugging)
  static bool show_collrects;
  static bool draw_solids_only;

  GameObjects gameobjects;
  MovingObjects moving_objects;
  SpawnPoints spawnpoints;
  Portables portables;

  std::string music;
  float gravity;

  // some special objects, where we need direct access
  // (try to avoid accessing them directly)
  std::vector<Player*> players;
  std::list<TileMap*> solid_tilemaps;
  Camera* camera;
  DisplayEffect* effect;
  
  SectorCollisionDetector collision_detector;

private:
  Sector(const Sector&);
  Sector& operator=(const Sector&);
};

#endif

/* EOF */
