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

#include "direction.h"
#include "math/vector.h"
#include "audio/musicref.h"
#include "video/drawing_context.h"

using namespace SuperTux;

namespace SuperTux {
class GameObject;
class Sprite;
class Rectangle;
}
namespace lisp {
class Lisp;
class Writer;
}

class Player;
class Camera;
class TileMap;
class Bullet;
class CollisionGrid;

class SpawnPoint
{
public:
  std::string name;
  Vector pos;
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

  void action(float elapsed_time);
  void update_game_objects();

  void draw(DrawingContext& context);

  /// adds a gameobject
  void add_object(GameObject* object);

  void set_name(const std::string& name)
  { this->name = name; }
  const std::string& get_name() const
  { return name; }

  /// tests if a given rectangle is inside the sector
  bool inside(const Rectangle& rectangle) const;

  void play_music(int musictype);
  int get_music_type();
  
  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void collision_handler();

  bool add_bullet(const Vector& pos, float xm, Direction dir);
  bool add_smoke_cloud(const Vector& pos);
  void add_floating_text(const Vector& pos, const std::string& text);
                                                                                
  /** @evil@ but can't always be avoided in current design... */
  static Sector* current()
  { return _current; }

  /** Get total number of badguys */
  int get_total_badguys();

  // make this private again soon
  void collision_tilemap(MovingObject* object, int depth);

private:
  void collision_object(MovingObject* object1, MovingObject* object2);
  
  void load_music();
  GameObject* parse_object(const std::string& name, const lisp::Lisp& lisp);
  
  static Sector* _current;
  
  std::string name;

  MusicRef level_song;
  MusicRef level_song_fast;

public:
  std::string song_title;
  float gravity;

  // some special objects, where we need direct access
  Player* player;
  TileMap* solids;
  Camera* camera;
  
private:
  std::vector<Bullet*> bullets;

public: // TODO make this private again
  typedef std::vector<GameObject*> GameObjects;
  GameObjects gameobjects;
  typedef std::vector<SpawnPoint*> SpawnPoints;
  SpawnPoints spawnpoints;                       

  Rectangle get_active_region();

private:
  void fix_old_tiles();
  
  /// container for newly created objects, they'll be added in Sector::action
  GameObjects gameobjects_new;
  
  int currentmusic;

  CollisionGrid* grid;
};

#endif

