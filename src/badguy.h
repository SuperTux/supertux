//
// Interface: badguy
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de> (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_BADGUY_H
#define SUPERTUX_BADGUY_H

#include "SDL.h"
#include "bitmask.h"
#include "type.h"
#include "timer.h"
#include "texture.h"
#include "physic.h"
#include "collision.h"

/* Enemy modes: */
#define NORMAL 0
#define FLAT 1
#define KICK 2
#define HELD 3

extern texture_type img_bsod_squished_left;
extern texture_type img_bsod_squished_right;
extern texture_type img_bsod_falling_left;
extern texture_type img_bsod_falling_right;
extern texture_type img_laptop_flat_left;
extern texture_type img_laptop_flat_right;
extern texture_type img_laptop_falling_left;
extern texture_type img_laptop_falling_right;
extern texture_type img_bsod_left[4];
extern texture_type img_bsod_right[4];
extern texture_type img_laptop_left[3];
extern texture_type img_laptop_right[3];
extern texture_type img_money_left[2];
extern texture_type img_money_right[2];

/* Bad guy kinds: */
enum BadGuyKind {
  BAD_BSOD,
  BAD_LAPTOP,
  BAD_MONEY
};

/* Badguy type: */
class BadGuy
{
 public:
  int mode;
  DyingType dying;
  BadGuyKind kind;
  bool seen;
  int dir;
  base_type base;
  base_type old_base;
  timer_type timer;
  physic_type physic;

 public:
  void init(float x, float y, BadGuyKind kind);

  void action();
  void draw();

  void action_bsod();
  void action_laptop();
  void action_money();

  void draw_bsod();
  void draw_laptop();
  void draw_money();

  void collision(void* p_c_object, int c_object);
};

#endif /*SUPERTUX_BADGUY_H*/


