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
enum {
    NORMAL=0,
    FLAT,
    KICK,
    HELD,

    BOMB_TICKING,
    BOMB_EXPLODE,

    STALACTITE_SHAKING,
    STALACTITE_FALL
};

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
extern texture_type img_mrbomb_left[4];
extern texture_type img_mrbomb_right[4];
extern texture_type img_stalactite;
extern texture_type img_stalactite_broken;

/* Bad guy kinds: */
enum BadGuyKind {
  BAD_BSOD,
  BAD_LAPTOP,
  BAD_MONEY,
  BAD_MRBOMB,
  BAD_BOMB,
  BAD_STALACTITE
};

BadGuyKind  badguykind_from_string(const std::string& str);
std::string badguykind_to_string(BadGuyKind kind);

struct BadGuyData
{
  BadGuyKind kind;
  int x;
  int y;

  BadGuyData(BadGuyKind kind_, int x_, int y_) 
    : kind(kind_), x(x_), y(y_) {}

  BadGuyData()
    : kind(BAD_BSOD), x(0), y(0) {}
};

class Player;

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

  void collision(void* p_c_object, int c_object,
          CollisionType type = COLLISION_NORMAL);
  
 private:
  void fall(bool dojump=false);
  void remove_me();

  void action_bsod();
  void draw_bsod();

  void action_laptop();
  void draw_laptop();
   
  void action_money(); 
  void draw_money();

  void action_bomb();
  void draw_bomb();

  void action_mrbomb();
  void draw_mrbomb();

  void action_stalactite();
  void draw_stalactite();

  void make_player_jump(Player* player);
  void check_horizontal_bump(bool checkcliff = false);
  void bump();
  void squich(Player* player);
};

#endif /*SUPERTUX_BADGUY_H*/


