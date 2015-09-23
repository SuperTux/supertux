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

#include "scripting/ssector.hpp"
#include "supertux/direction.hpp"
#include "supertux/game_object_ptr.hpp"
#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"
#include "util/currenton.hpp"
#include "video/color.hpp"
#include "object/anchor_point.hpp"

namespace collision {
class Constraints;
}

class Vector;
class Rectf;
class Sprite;
class GameObject;
class Player;
class PlayerStatus;
class Camera;
class TileMap;
class Bullet;
class ScriptInterpreter;
class SpawnPoint;
class MovingObject;
class CollisionHit;
class Level;
class Portable;
class DrawingContext;
class DisplayEffect;

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
class Sector : public scripting::SSector,
               public Currenton<Sector>
{
public:
  Sector(Level* parent);
  ~Sector();

  /// get parent level
  Level* get_level() const;

  /// read sector from lisp file
  void parse(const Reader& lisp);
  void parse_old_format(const Reader& lisp);

  /// activates this sector (change music, initialize player class, ...)
  void activate(const std::string& spawnpoint);
  void activate(const Vector& player_pos);
  void deactivate();

  void update(float elapsed_time);
  void update_game_objects();

  void draw(DrawingContext& context);

  /**
   * runs a script in the context of the sector (sector_table will be the
   * roottable of this squirrel VM)
   */
  HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename);

  /// adds a gameobject
  void add_object(GameObjectPtr object);

  void set_name(const std::string& name_)
  { this->name = name_; }
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
  template<class T> int get_total_count()
  {
    int total = 0;
    for(GameObjects::iterator i = gameobjects.begin(); i != gameobjects.end(); ++i) {
      if (dynamic_cast<T*>(i->get())) total++;
    }
    return total;
  }

  void collision_tilemap(collision::Constraints* constraints,
                         const Vector& movement, const Rectf& dest,
                         MovingObject &object) const;

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
  bool is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object = 0, const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC, COLGROUP_MOVINGSTATIC or COLGROUP_MOVING.
   * This includes badguys and players.
   */
  bool is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object = 0) const;

  bool free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object = 0) const;
  bool can_see_player(const Vector& eye) const;

/**
   * returns a list of players currently in the sector
   */
  std::vector<Player*> get_players() const {
    return std::vector<Player*>(1, this->player);
  }
  Player *get_nearest_player (const Vector& pos) const;
  Player *get_nearest_player (const Rectf& pos) const
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
  float get_ambient_red();
  float get_ambient_green();
  float get_ambient_blue();

  /**
   *  set gravity throughout sector
   */
  void set_gravity(float gravity);
  float get_gravity() const;

private:
  uint32_t collision_tile_attributes(const Rectf& dest) const;

  void before_object_remove(GameObjectPtr object);
  bool before_object_add(GameObjectPtr object);

  void try_expose(GameObjectPtr object);
  void try_unexpose(GameObjectPtr object);
  void try_expose_me();
  void try_unexpose_me();

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

  GameObjectPtr parse_object(const std::string& name, const Reader& lisp);

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
  Player* player;
  std::list<TileMap*> solid_tilemaps;
  Camera* camera;
  DisplayEffect* effect;

private:
  Sector(const Sector&);
  Sector& operator=(const Sector&);
};

#endif

/* EOF */
