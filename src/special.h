//
// C Interface: special
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#define BULLET_STARTING_YM 1
#define BULLET_XM 5

#ifndef SUPERTUX_SPECIAL_H
#define SUPERTUX_SPECIAL_H

#include <SDL.h>
#include "bitmask.h"
#include "type.h"
#include "texture.h"
#include "collision.h"
#include "player.h"

/* Upgrade types: */

enum {
  UPGRADE_MINTS,
  UPGRADE_COFFEE,
  UPGRADE_HERRING
};

class Upgrade
{
 public:
  int kind;
  base_type base;
  base_type old_base;

  void init(float x, float y, int dir, int kind);
  void action(double frame_ratio);
  void draw();
  void collision(void* p_c_object, int c_object);
};

class Bullet
{
 public:
  base_type base;
  base_type old_base;
  
  void init(float x, float y, float xm, int dir);
  void action(double frame_ratio);
  void draw();
  void collision(int c_object);
};

extern texture_type img_bullet;
extern bitmask* bm_bullet;

void create_special_bitmasks();

extern texture_type img_golden_herring;

#endif /*SUPERTUX_SPECIAL_H*/
