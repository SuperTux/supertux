//
// Interface: player/tux
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_PLAYER_H
#define SUPERTUX_PLAYER_H

#include <SDL.h>
#include "bitmask.h"
#include "type.h"
#include "timer.h"
#include "texture.h"
#include "collision.h"
#include "sound.h"
#include "physic.h"

/* Times: */

#define TUX_SAFE_TIME 750
#define TUX_INVINCIBLE_TIME 10000
#define TIME_WARNING 20000     /* When to alert player they're low on time! */

/* One-ups... */

#define DISTROS_LIFEUP 100

/* Scores: */

#define SCORE_BRICK 5
#define SCORE_DISTRO 25

#include <vector>

struct player_keymap_type
{
  int jump;
  int duck;
  int left;
  int right;
  int fire;
};

struct player_input_type
{
  int right;
  int left;
  int up;
  int down;
  int fire;
  int old_fire;
};

void player_input_init(player_input_type* pplayer_input);

extern Surface* tux_life;
extern std::vector<Surface*> tux_right;
extern std::vector<Surface*> tux_left;
extern Surface* smalltux_jump_left;
extern Surface* smalltux_jump_right;
extern Surface* smalltux_stand_left;
extern Surface* smalltux_stand_right;
extern Surface* bigtux_right[3];
extern Surface* bigtux_left[3];
extern Surface* bigtux_right_jump;
extern Surface* bigtux_left_jump;
extern Surface* ducktux_right;
extern Surface* ducktux_left;
extern Surface* skidtux_right;
extern Surface* skidtux_left;
extern Surface* firetux_right[3];
extern Surface* firetux_left[3];
extern Surface* bigfiretux_right[3];
extern Surface* bigfiretux_left[3];
extern Surface* bigfiretux_right_jump;
extern Surface* bigfiretux_left_jump;
extern Surface* duckfiretux_right;
extern Surface* duckfiretux_left;
extern Surface* skidfiretux_right;
extern Surface* skidfiretux_left;
extern Surface* cape_right[2];
extern Surface* cape_left[2];
extern Surface* bigcape_right[2];
extern Surface* bigcape_left[2];

class Player
{
 public:
  player_keymap_type keymap;

  player_input_type  input;
  bool got_coffee;
  int size;
  bool duck;
  DyingType dying;
  int dir;
  bool jumping;
  int frame_;
  int frame_main;

  base_type  base;
  base_type  old_base;
  base_type  previous_base;
  Timer invincible_timer;
  Timer skidding_timer;
  Timer safe_timer;
  Timer frame_timer;
  Physic physic;

 public:
  void init();
  int  key_event(SDLKey key, int state);
  void level_begin();
  void action(double frame_ratio);
  void handle_input();
  void grabdistros();
  void draw();
  void collision(void* p_c_object, int c_object);
  void kill(int mode);
  void is_dying();
  bool is_dead();
  void player_remove_powerups();
  void keep_in_bounds();
  bool on_ground();
  bool under_solid();
  
 private:
  void handle_horizontal_input();
  void handle_vertical_input();
  void remove_powerups();
};

#endif /*SUPERTUX_PLAYER_H*/
