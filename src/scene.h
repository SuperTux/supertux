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
int score, highscore, distros, level, next_level, game_pause,
done, quit, score_multiplier, super_bkgd_time, endpos,
counting_distros, distro_counter;
float scroll_x;
int frame;
bouncy_distro_type *bouncy_distros;
broken_brick_type *broken_bricks;
bouncy_brick_type *bouncy_bricks;
bad_guy_type *bad_guys;
floating_score_type *floating_scores;
upgrade_type *upgrades;
bullet_type *bullets;
int num_bad_guys;
int num_bouncy_distros;
int num_broken_bricks;
int num_bouncy_bricks;
int num_floating_scores;
int num_upgrades;
int num_bullets;
player_type tux;
SDL_Rect src, dest;
texture_type img_box_full, img_box_empty, img_mints, img_coffee, img_super_bkgd, img_red_glow;
st_level current_level;

void add_score(int x, int y, int s);
void set_defaults(void);
void arrays_init(void);
void arrays_free(void);

#endif /*SUPERTUX_SCENE_H*/
