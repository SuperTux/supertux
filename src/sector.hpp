//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#include <string>
#include <vector>

#include "direction.hpp"
#include "math/vector.hpp"
#include "video/drawing_context.hpp"

namespace lisp {
class Lisp;
class Writer;
}

class Rect;
class Sprite;
class GameObject;
class Player;
class Camera;
class TileMap;
class Bullet;
class CollisionGrid;
class ScriptInterpreter;
class SpawnPoint;
class MovingObject;
class CollisionHit;

enum MusicType {
  LEVEL_MUSIC,
  HERRING_MUSIC
};

/** This class holds a sector (a part of a level) and all the game objects
 * (badguys, player, background, tilemap, ...)
 */
class Sector
{
public:
  Sector();
  ~Sector();

  /// read sector from lisp file
  void parse(const lisp::Lisp& lisp);
  void parse_old_format(const lisp::Lisp& lisp);
  /// write sector to lisp file
  void write(lisp::Writer& writer);

  /// activates this sector (change music, intialize player class, ...)
  void activate(const std::string& spawnpoint);
  void activate(const Vector& player_pos);

  void update(float elapsed_time);
  void update_game_objects();

  void draw(DrawingContext& context);

  /// adds a gameobject
  void add_object(GameObject* object);

  void set_name(const std::string& name)
  { this->name = name; }
  const std::string& get_name() const
  { return name; }

  /// tests if a given rectangle is inside the sector
  bool inside(const Rect& rectangle) const;

  void play_music(MusicType musictype);
  MusicType get_music_type();
  
  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void handle_collisions();

  bool add_bullet(const Vector& pos, float xm, Direction dir);
  bool add_smoke_cloud(const Vector& pos);
  void add_floating_text(const Vector& pos, const std::string& text);
                                                                                
  /** get currently activated sector. */
  static Sector* current()
  { return _current; }

  /** Get total number of badguys */
  int get_total_badguys();

  void collision_tilemap(MovingObject* object, CollisionHit& hit) const;
  uint32_t collision_tile_attributes(MovingObject* object) const;

  /** Checks if at the specified rectangle are gameobjects with STATIC flag set
   * (or solid tiles from the tilemap)
   */
  bool is_free_space(const Rect& rect) const;

  /**
   * returns a list of players currently in the sector
   */
  std::vector<Player*> get_players() {
    return std::vector<Player*>(1, this->player);
  }

private:
  void collision_object(MovingObject* object1, MovingObject* object2) const;
  GameObject* parse_object(const std::string& name, const lisp::Lisp& lisp);
  
  static Sector* _current;
  
  std::string name;

public:
  std::string music;
  float gravity;

  // some special objects, where we need direct access
  Player* player;
  TileMap* solids;
  Camera* camera;
  
private:
  std::vector<Bullet*> bullets;

  std::string init_script;

public: // TODO make this private again
  typedef std::vector<GameObject*> GameObjects;
  GameObjects gameobjects;
  typedef std::vector<MovingObject*> MovingObjects;
  MovingObjects moving_objects;
  typedef std::vector<SpawnPoint*> SpawnPoints;
  SpawnPoints spawnpoints;                       

  Rect get_active_region();

private:
  void fix_old_tiles();
  
  /// container for newly created objects, they'll be added in Sector::update
  GameObjects gameobjects_new;
 
  MusicType currentmusic;

  CollisionGrid* grid;
};

#endif

