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

#include "math/vector.h"
#include "badguy.h"
#include "special.h"
#include "audio/musicref.h"
#include "video/drawing_context.h"

using namespace SuperTux;

namespace SuperTux {
class GameObject;
class LispReader;
}

class InteractiveObject;
class Background;
class Player;
class Camera;
class Trampoline;
class FlyingPlatform;
class TileMap;
class Upgrade;
class Bullet;
class SmokeCloud;
class Particles;
class BadGuy;
class Tile;

struct SpawnPoint
{
  std::string name;
  Vector pos;
};

enum {
  NONE_ENDSEQ_ANIM,
  FIREWORKS_ENDSEQ_ANIM
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
  void parse(LispReader& reader);
  void parse_old_format(LispReader& reader);
  /// write sector to lisp file
  void write(LispWriter& writer);

  /// activates this sector (change music, intialize player class, ...)
  void activate(const std::string& spawnpoint = "main");
  /// get best spawn point
  Vector get_best_spawn_point(Vector pos);

  void action(float elapsed_time);
  void update_game_objects();

  void draw(DrawingContext& context);

  /// adds a gameobject
  void add_object(GameObject* object);

  const std::string& get_name() const
  { return name; }

  void play_music(int musictype);
  int get_music_type();
  
  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void collision_handler();
                                                                                
  void add_score(const Vector& pos, int s);
  void add_bouncy_distro(const Vector& pos);
  void add_broken_brick(const Vector& pos, Tile* tile);
  void add_broken_brick_piece(const Vector& pos,
      const Vector& movement, Tile* tile);
  void add_bouncy_brick(const Vector& pos);
                                                                                
  BadGuy* add_bad_guy(float x, float y, BadGuyKind kind);
                                                                                
  void add_upgrade(const Vector& pos, Direction dir, UpgradeKind kind);
  bool add_bullet(const Vector& pos, float xm, Direction dir);
  bool add_smoke_cloud(const Vector& pos);
  bool add_particles(const Vector& epicenter, const Vector& velocity, const Vector& acceleration, int number, Color color, int size, int life_time);
                                                                                
  /** Try to grab the coin at the given coordinates */
  void trygrabdistro(const Vector& pos, int bounciness);
                                                                                
  /** Try to break the brick at the given coordinates */
  bool trybreakbrick(const Vector& pos, bool small);
                                                                                
  /** Try to get the content out of a bonus box, thus emptying it */
  void tryemptybox(const Vector& pos, Direction col_side);
                                                                                
  /** Try to bumb a badguy that might we walking above Tux, thus shaking
      the tile which the badguy is walking on an killing him this way */
  void trybumpbadguy(const Vector& pos);

  /** Flip the all the sector vertically. The purpose of this is to let
      player to play the same level in a different way :) */
  void do_vertical_flip();

  /** Get end sequence animation */
  int end_sequence_animation()
    { return end_sequence_animation_type; }

  /** @evil@ */
  static Sector* current()
  { return _current; }

private:
  void load_music();
  
  static Sector* _current;
  
  std::string name;

  MusicRef level_song;
  MusicRef level_song_fast;

  int end_sequence_animation_type;

public:
  std::string song_title;
  float gravity;

  // some special objects, where we need direct access
  Player* player;
  TileMap* solids;
  Background* background;
  Camera* camera;
  
private:
  typedef std::vector<BadGuy*> BadGuys;
  BadGuys badguys;
  typedef std::vector<Trampoline*> Trampolines;
  Trampolines trampolines;
  typedef std::vector<FlyingPlatform*> FlyingPlatforms;
  FlyingPlatforms flying_platforms;

  std::vector<Upgrade*> upgrades;
  std::vector<Bullet*> bullets;
  std::vector<SmokeCloud*> smoke_clouds;
  std::vector<Particles*> particles;

public: // ugly
  typedef std::vector<InteractiveObject*> InteractiveObjects;
  InteractiveObjects interactive_objects;
  typedef std::vector<GameObject*> GameObjects;
  GameObjects gameobjects;
  GameObjects gameobjects_new; // For newly created objects

private:
  typedef std::vector<SpawnPoint*> SpawnPoints;
  SpawnPoints spawnpoints;

  int distro_counter;
  bool counting_distros;
  int currentmusic;        
};

#endif

