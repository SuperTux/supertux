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
#include "physic.h"

/* Upgrade types: */

enum {
  UPGRADE_GROWUP,
  UPGRADE_ICEFLOWER,
  UPGRADE_HERRING,
  UPGRADE_1UP
};

void load_special_gfx();
void free_special_gfx();

class Upgrade
{
public:
  int kind;
  int dir;
  base_type base;
  base_type old_base;
  Physic physic;

  void init(float x, float y, int dir, int kind);
  void action(double frame_ratio);
  void draw();
  void collision(void* p_c_object, int c_object);

private:
  /** removes the Upgrade from the global upgrade list. Note that after this
   * call the class doesn't exist anymore! So don't use any member variables
   * anymore then
   */
  void remove_me();
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

private:
  /** removes the Upgrade from the global upgrade list. Note that after this
   * call the class doesn't exist anymore! So don't use any member variables
   * anymore then
   */
  void remove_me();
};

#endif /*SUPERTUX_SPECIAL_H*/
