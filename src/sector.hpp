//  SuperTux -  A Jump'n Run
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#ifndef SUPERTUX_SECTOR_H
#define SUPERTUX_SECTOR_H

#include <vector>
#include <list>
#include <memory>
#include <squirrel.h>
#include <stdint.h>

#include "direction.hpp"
#include "video/color.hpp"
#include "scripting/ssector.hpp"

namespace lisp {
class Lisp;
class Writer;
}
namespace collision {
class Constraints;
}

class Vector;
class Rect;
class Sprite;
class GameObject;
class Player;
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
class Sector : public Scripting::SSector
{
public:
  Sector(Level* parent);
  ~Sector();

  /// get parent level
  Level* get_level();

  /// read sector from lisp file
  void parse(const lisp::Lisp& lisp);
  void parse_old_format(const lisp::Lisp& lisp);
  /// write sector to lisp file
  void write(lisp::Writer& writer);

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
  void add_object(GameObject* object);

  void set_name(const std::string& name)
  { this->name = name; }
  const std::string& get_name() const
  { return name; }

  /**
   * tests if a given rectangle is inside the sector
   * (a rectangle that is on top of the sector is considered inside)
   */
  bool inside(const Rect& rectangle) const;

  void play_music(MusicType musictype);
  MusicType get_music_type();

  bool add_bullet(const Vector& pos, float xm, Direction dir);
  bool add_smoke_cloud(const Vector& pos);

  /** get currently activated sector. */
  static Sector* current()
  { return _current; }

  /** Get total number of badguys */
  int get_total_badguys();

  /** Get total number of GameObjects of given type */
  template<class T> int get_total_count()
  {
    int total = 0;
    for(GameObjects::iterator i = gameobjects.begin(); i != gameobjects.end(); ++i) {
      if (dynamic_cast<T*>(*i)) total++;
    }
    return total;
  }

  void collision_tilemap(collision::Constraints* constraints,
      const Vector& movement, const Rect& dest) const;

  /**
   * Checks if the specified rectangle is free of (solid) tiles.
   * Note that this does not include static objects, e.g. bonus blocks.
   */
  bool is_free_of_tiles(const Rect& rect, const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC.
   * Note that this does not include badguys or players.
   */
  bool is_free_of_statics(const Rect& rect, const MovingObject* ignore_object = 0, const bool ignoreUnisolid = false) const;
  /**
   * Checks if the specified rectangle is free of both
   * 1.) solid tiles and
   * 2.) MovingObjects in COLGROUP_STATIC, COLGROUP_MOVINGSTATIC or COLGROUP_MOVING.
   * This includes badguys and players.
   */
  bool is_free_of_movingstatics(const Rect& rect, const MovingObject* ignore_object = 0) const;

  /**
   * returns a list of players currently in the sector
   */
  std::vector<Player*> get_players() {
    return std::vector<Player*>(1, this->player);
  }

  Rect get_active_region();

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

  typedef std::vector<GameObject*> GameObjects;
  typedef std::vector<MovingObject*> MovingObjects;
  typedef std::vector<SpawnPoint*> SpawnPoints;
  typedef std::vector<Portable*> Portables;

  // --- Scripting ---
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

private:
  Level* level; /**< Parent level containing this sector */
  uint32_t collision_tile_attributes(const Rect& dest) const;

  void before_object_remove(GameObject* object);
  bool before_object_add(GameObject* object);

  void try_expose(GameObject* object);
  void try_unexpose(GameObject* object);
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
      const Vector& movement, const Rect& dest, GameObject& object);

  void collision_static_constrains(MovingObject& object);

  GameObject* parse_object(const std::string& name, const lisp::Lisp& lisp);

  void fix_old_tiles();

  static Sector* _current;

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
};

#endif
