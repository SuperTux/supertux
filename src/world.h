//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

class Level;

/** The World class holds a level and all the game objects (badguys,
    bouncy distros, etc) that are needed to run a game. */
class World
{
 public:
  Level* level;

  Player tux;
  
  std::vector<BouncyDistro> bouncy_distros;
  std::vector<BrokenBrick>  broken_bricks;
  std::vector<BouncyBrick>  bouncy_bricks;
  std::vector<FloatingScore> floating_scores;

  std::vector<BadGuy> bad_guys;
  std::vector<Upgrade> upgrades;
  std::vector<Bullet> bullets;
  std::vector<ParticleSystem*> particle_systems;

  int distro_counter;
  bool counting_distros;

  static World* current_;
 public:
  static World* current() { return current_; }
  static void set_current(World* w) { current_ = w; }

  World();
  ~World();
  
  Level*  get_level() { return level; }
  Player* get_tux() { return &tux; }

  void set_defaults();

  void draw();
  void action(double frame_ratio);

  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void collision_handler();
  
  void arrays_free();

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const std::string& subset, int level);

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const std::string& filename);

  void activate_particle_systems();
  void activate_bad_guys();

  void add_score(float x, float y, int s);
  void add_bouncy_distro(float x, float y);
  void add_broken_brick(Tile* tile, float x, float y);
  void add_broken_brick_piece(Tile* tile, float x, float y, float xm, float ym);
  void add_bouncy_brick(float x, float y);
  void add_bad_guy(float x, float y, BadGuyKind kind);
  void add_upgrade(float x, float y, int dir, int kind);
  void add_bullet(float x, float y, float xm, int dir);

  /** Try to grab the coin at the given coordinates */
  void trygrabdistro(float x, float y, int bounciness);

  /** Try to break the brick at the given coordinates */
  void trybreakbrick(float x, float y, bool small);

  /** Try to get the content out of a bonus box, thus emptying it */
  void tryemptybox(float x, float y, int col_side);

  /** Try to bumb a badguy that might we walking above Tux, thus shaking
      the tile which the badguy is walking on an killing him this way */
  void trybumpbadguy(float x, float y);
};

/** FIMXE: Workaround for the leveleditor mainly */
extern World global_world;

#endif /*SUPERTUX_WORLD_H*/

/* Local Variables: */
/* mode:c++ */
/* End */

