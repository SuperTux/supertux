//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef SUPERTUX_WORLD_H
#define SUPERTUX_WORLD_H

#include <vector>
#include <SDL.h>
#include "type.h"
#include "scene.h"
#include "special.h"
#include "badguy.h"
#include "particlesystem.h"
#include "gameobjs.h"
#include "display_manager.h"

class Level;
class Background;

/** The World class holds a level and all the game objects (badguys,
    bouncy distros, etc) that are needed to run a game. */
class World
{
private:
  typedef std::list<BadGuy*> BadGuys;
  BadGuys bad_guys_to_add;
  typedef std::list<Trampoline*> Trampolines;
  Trampolines trampolines;
  Level* level;
  Player* tux;

  Timer scrolling_timer;

  int distro_counter;
  bool counting_distros;
  int currentmusic;

  static World* current_;
public:
  Background* background;
  BadGuys bad_guys;

  std::vector<Upgrade*> upgrades;
  std::vector<Bullet*> bullets;
  std::vector<GameObject*> gameobjects;

  DisplayManager displaymanager;

public:
  static World* current() { return current_; }
  static void set_current(World* w) { current_ = w; }

  World(const std::string& filename);
  World(const std::string& subset, int level_nr);
  //World() {};
  ~World();
  
  Level*  get_level() { return level; }
  Player* get_tux() { return tux; }

  void add_object(GameObject* object);

  void set_defaults();

  void draw();
  void action(float elapsed_time);
  void scrolling(float elapsed_time);   // camera scrolling

  void play_music(int musictype);
  int get_music_type();

  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void collision_handler();

  void parse_objects(lisp_object_t* cur);
  
  void activate_particle_systems();

  void add_score(const Vector& pos, int s);
  void add_bouncy_distro(const Vector& pos);
  void add_broken_brick(const Vector& pos, Tile* tile);
  void add_broken_brick_piece(const Vector& pos,
      const Vector& movement, Tile* tile);
  void add_bouncy_brick(const Vector& pos);

  BadGuy* add_bad_guy(float x, float y, BadGuyKind kind);

  void add_upgrade(const Vector& pos, Direction dir, UpgradeKind kind);
  bool add_bullet(const Vector& pos, float xm, Direction dir);

  /** Try to grab the coin at the given coordinates */
  void trygrabdistro(float x, float y, int bounciness);

  /** Try to break the brick at the given coordinates */
  bool trybreakbrick(float x, float y, bool small);

  /** Try to get the content out of a bonus box, thus emptying it */
  void tryemptybox(float x, float y, Direction col_side);

  /** Try to bumb a badguy that might we walking above Tux, thus shaking
      the tile which the badguy is walking on an killing him this way */
  void trybumpbadguy(float x, float y);

  /** Apply bonuses active in the player status, used to reactivate
      bonuses from former levels */
  void apply_bonuses();
};

/** FIMXE: Workaround for the leveleditor mainly */
extern World global_world;

#endif /*SUPERTUX_WORLD_H*/

/* Local Variables: */
/* mode:c++ */
/* End: */

