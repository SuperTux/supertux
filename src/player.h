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

extern texture_type tux_life;
extern texture_type tux_right[3];
extern texture_type tux_left[3];
extern texture_type bigtux_right[3];
extern texture_type bigtux_left[3];
extern texture_type bigtux_right_jump;
extern texture_type bigtux_left_jump;
extern texture_type ducktux_right;
extern texture_type ducktux_left;
extern texture_type skidtux_right;
extern texture_type skidtux_left;
extern texture_type firetux_right[3];
extern texture_type firetux_left[3];
extern texture_type bigfiretux_right[3];
extern texture_type bigfiretux_left[3];
extern texture_type bigfiretux_right_jump;
extern texture_type bigfiretux_left_jump;
extern texture_type duckfiretux_right;
extern texture_type duckfiretux_left;
extern texture_type skidfiretux_right;
extern texture_type skidfiretux_left;
extern texture_type cape_right[2];
extern texture_type cape_left[2];
extern texture_type bigcape_right[2];
extern texture_type bigcape_left[2];

class Player
{
 public:
  player_input_type  input;
  player_keymap_type keymap;
  int score;
  int distros;
  bool got_coffee;
  int size;
  bool duck;
  DyingType dying;
  int dir;
  bool jumping;
  int frame_;
  int frame_main;
  int lives;
  base_type base;
  base_type old_base;
  base_type previous_base;
  timer_type invincible_timer;
  timer_type skidding_timer;
  timer_type safe_timer;
  timer_type frame_timer;
  physic_type vphysic;
  physic_type hphysic;

 public:
  void init();
  int  key_event(SDLKey key, int state);
  void level_begin();
  void action();
  void handle_input();
  void grabdistros();
  void draw();
  void collision(void* p_c_object, int c_object);
  void kill(int mode);
  void is_dying();
  void player_remove_powerups();
  void keep_in_bounds();
  bool on_ground();
  bool under_solid();
 private:
  void handle_horizontal_input(int dir);
  void handle_vertical_input();
  void remove_powerups();
};

#endif /*SUPERTUX_PLAYER_H*/
