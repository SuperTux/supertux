//
// C Interface: scene
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_SCENE_H
#define SUPERTUX_SCENE_H

#include "defines.h"
#include "gameloop.h"
#include "player.h"
#include "badguy.h"
#include "world.h"
#include "special.h"
#include "level.h"

#define FRAME_RATE 10 // 100 Frames per second (10ms)
extern int score, distros, level, next_level, game_pause, quit, score_multiplier, endpos, counting_distros, distro_counter;
extern timer_type  super_bkgd_timer;
extern float scroll_x;
extern int frame;
extern bouncy_distro_type *bouncy_distros;
extern broken_brick_type *broken_bricks;
extern bouncy_brick_type *bouncy_bricks;
extern bad_guy_type *bad_guys;
extern floating_score_type *floating_scores;
extern upgrade_type *upgrades;
extern bullet_type *bullets;
extern int num_bad_guys;
extern int num_bouncy_distros;
extern int num_broken_bricks;
extern int num_bouncy_bricks;
extern int num_floating_scores;
extern int num_upgrades;
extern int num_bullets;
extern player_type tux;
extern texture_type img_box_full, img_box_empty, img_mints, img_coffee, img_super_bkgd, img_red_glow;
extern timer_type time_left;
extern double frame_ratio;

void add_score(float x, float y, int s);
void set_defaults(void);
void arrays_init(void);
void arrays_free(void);

void add_bouncy_distro(float x, float y);
void add_broken_brick(float x, float y);
void add_broken_brick_piece(float x, float y, float xm, float ym);
void add_bouncy_brick(float x, float y);
void add_bad_guy(float x, float y, int kind);
void add_upgrade(float x, float y, int dir, int kind);
void add_bullet(float x, float y, float xm, int dir);

#endif /*SUPERTUX_SCENE_H*/
