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

#include <SDL.h>
#include "type.h"

/* Bounciness of distros: */

#define NO_BOUNCE 0
#define BOUNCE 1

typedef struct bouncy_distro_type /*It is easier to read the sources IMHO, if we don't write something like int a,b,c; */
  {
    base_type base;
  }
bouncy_distro_type;

extern texture_type img_distro[4];

void bouncy_distro_init(bouncy_distro_type* pbouncy_distro, float x, float y);
void bouncy_distro_action(bouncy_distro_type* pbouncy_distro);
void bouncy_distro_draw(bouncy_distro_type* pbouncy_distro);
void bouncy_distro_collision(bouncy_distro_type* pbouncy_distro, int c_object);

#define BOUNCY_BRICK_MAX_OFFSET 8
#define BOUNCY_BRICK_SPEED 0.9

typedef struct broken_brick_type
  {
    base_type base;
    timer_type timer;
  }
broken_brick_type;

void broken_brick_init(broken_brick_type* pbroken_brick, float x, float y, float xm, float ym);
void broken_brick_action(broken_brick_type* pbroken_brick);
void broken_brick_draw(broken_brick_type* pbroken_brick);

typedef struct bouncy_brick_type
  {
    float offset;
    float offset_m;
    int shape;
    base_type base;
  }
bouncy_brick_type;

void bouncy_brick_init(bouncy_brick_type* pbouncy_brick, float x, float y);
void bouncy_brick_action(bouncy_brick_type* pbouncy_brick);
void bouncy_brick_draw(bouncy_brick_type* pbouncy_brick);

typedef struct floating_score_type
  {
    int value;
    timer_type timer;
    base_type base;
  }
floating_score_type;

void floating_score_init(floating_score_type* pfloating_score, float x, float y, int s);
void floating_score_action(floating_score_type* pfloating_score);
void floating_score_draw(floating_score_type* pfloating_score);

#endif /*SUPERTUX_WORLD_H*/

