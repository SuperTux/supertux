
#ifndef SUPERTUX_GAMEOBJS_H
#define SUPERTUX_GAMEOBJS_H

#include "type.h"
#include "texture.h"
#include "timer.h"
#include "scene.h"

/* Bounciness of distros: */
#define NO_BOUNCE 0
#define BOUNCE 1

class bouncy_distro_type
{
 public:
  base_type base;
};

extern texture_type img_distro[4];

void bouncy_distro_init(bouncy_distro_type* pbouncy_distro, float x, float y);
void bouncy_distro_action(bouncy_distro_type* pbouncy_distro);
void bouncy_distro_draw(bouncy_distro_type* pbouncy_distro);
void bouncy_distro_collision(bouncy_distro_type* pbouncy_distro, int c_object);

#define BOUNCY_BRICK_MAX_OFFSET 8
#define BOUNCY_BRICK_SPEED 0.9

class Tile;

class broken_brick_type
{
 public:
  base_type base;
  timer_type timer;
  Tile* tile;
};

void broken_brick_init(broken_brick_type* pbroken_brick, Tile* tile,
                       float x, float y, float xm, float ym);
void broken_brick_action(broken_brick_type* pbroken_brick);
void broken_brick_draw(broken_brick_type* pbroken_brick);

class bouncy_brick_type
{
 public:
  float offset;
  float offset_m;
  int shape;
  base_type base;
};

void bouncy_brick_init(bouncy_brick_type* pbouncy_brick, float x, float y);
void bouncy_brick_action(bouncy_brick_type* pbouncy_brick);
void bouncy_brick_draw(bouncy_brick_type* pbouncy_brick);

class floating_score_type
{
 public:
  int value;
  timer_type timer;
  base_type base;
};

void floating_score_init(floating_score_type* pfloating_score, float x, float y, int s);
void floating_score_action(floating_score_type* pfloating_score);
void floating_score_draw(floating_score_type* pfloating_score);

#endif 

/* Local Variables: */
/* mode:c++ */
/* End */
