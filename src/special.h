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

typedef struct upgrade_type
  {
    int kind;
    base_type base;
    base_type old_base;
  }
upgrade_type;

typedef struct bullet_type
  {
    base_type base;
    base_type old_base;
  }
bullet_type;

extern texture_type img_bullet;
extern bitmask* bm_bullet;

void create_special_bitmasks();

extern texture_type img_golden_herring;

void upgrade_init(upgrade_type *pupgrade, float x, float y, int dir, int kind);
void upgrade_action(upgrade_type *pupgrade);
void upgrade_draw(upgrade_type *pupgrade);
void upgrade_collision(upgrade_type *pupgrade, void* p_c_object, int c_object);

void bullet_init(bullet_type* pbullet, float x, float y, float xm, int dir);
void bullet_action(bullet_type *pbullet);
void bullet_draw(bullet_type *pbullet);
void bullet_collision(bullet_type *pbullet, int c_object);

#endif /*SUPERTUX_SPECIAL_H*/
