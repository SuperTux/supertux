//
// Interface: world
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_WORLD_H
#define SUPERTUX_WORLD_H

#include <vector>
#include <SDL.h>
#include "type.h"
#include "scene.h"
#include "special.h"
#include "particlesystem.h"

/* Bounciness of distros: */

#define NO_BOUNCE 0
#define BOUNCE 1

struct bouncy_distro_type
{
  base_type base;
};

extern texture_type img_distro[4];

void bouncy_distro_init(bouncy_distro_type* pbouncy_distro, float x, float y);
void bouncy_distro_action(bouncy_distro_type* pbouncy_distro);
void bouncy_distro_draw(bouncy_distro_type* pbouncy_distro);
void bouncy_distro_collision(bouncy_distro_type* pbouncy_distro, int c_object);

#define BOUNCY_BRICK_MAX_OFFSET 8
#define BOUNCY_BRICK_SPEED 0.9

struct broken_brick_type
{
  base_type base;
  timer_type timer;
  Tile* tile;
};

void broken_brick_init(broken_brick_type* pbroken_brick, Tile* tile,
                       float x, float y, float xm, float ym);
void broken_brick_action(broken_brick_type* pbroken_brick);
void broken_brick_draw(broken_brick_type* pbroken_brick);

struct bouncy_brick_type
{
  float offset;
  float offset_m;
  int shape;
  base_type base;
};

void bouncy_brick_init(bouncy_brick_type* pbouncy_brick, float x, float y);
void bouncy_brick_action(bouncy_brick_type* pbouncy_brick);
void bouncy_brick_draw(bouncy_brick_type* pbouncy_brick);

struct floating_score_type
{
  int value;
  timer_type timer;
  base_type base;
};

void floating_score_init(floating_score_type* pfloating_score, float x, float y, int s);
void floating_score_action(floating_score_type* pfloating_score);
void floating_score_draw(floating_score_type* pfloating_score);

/** Try to grab the coin at the given coordinates */
void trygrabdistro(float x, float y, int bounciness);

/** Try to break the brick at the given coordinates */
void trybreakbrick(float x, float y, bool small);

/** Try to get the content out of a bonus box, thus emptying it */
void tryemptybox(float x, float y, int col_side);

/** Try to bumb a badguy that might we walking above Tux, thus shaking
    the tile which the badguy is walking on an killing him this way */
void trybumpbadguy(float x, float y);


/** The World class holds a level and all the game objects (badguys,
    bouncy distros, etc) that are needed to run a game. */
class World
{
 public:
  Level* level;
  
  std::vector<bouncy_distro_type> bouncy_distros;
  std::vector<broken_brick_type> broken_bricks;
  std::vector<bouncy_brick_type> bouncy_bricks;
  std::vector<BadGuy> bad_guys;
  std::vector<floating_score_type> floating_scores;
  std::vector<upgrade_type> upgrades;
  std::vector<bullet_type> bullets;
  std::vector<ParticleSystem*> particle_systems;

 public:
  World();
  ~World();
  
  Level* get_level() { return level; }

  void draw();
  void action();
  void arrays_free();

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const char* subset, int level);

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const std::string& filename);

  void activate_particle_systems();

  void add_score(float x, float y, int s);
  void add_bouncy_distro(float x, float y);
  void add_broken_brick(Tile* tile, float x, float y);
  void add_broken_brick_piece(Tile* tile, float x, float y, float xm, float ym);
  void add_bouncy_brick(float x, float y);
  void add_bad_guy(float x, float y, BadGuyKind kind);
  void add_upgrade(float x, float y, int dir, int kind);
  void add_bullet(float x, float y, float xm, int dir);
};

extern World world;

#endif /*SUPERTUX_WORLD_H*/

