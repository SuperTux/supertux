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

typedef struct player_keymap_type
{
 int jump;
 int duck;
 int left;
 int right;
 int fire;
}
player_keymap_type;

typedef struct player_input_type
{
 int right;
 int left;
 int up;
 int down;
 int fire;
 int old_fire;
}
player_input_type;

void player_input_init(player_input_type* pplayer_input);

typedef struct player_type 
{
 player_input_type input;
 player_keymap_type keymap;
 int score;
 int distros;
 int got_coffee;
 int size;
 int duck;
 int dying;
 int dir;
 int jumping;
 int frame_main;
 int frame;
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
}
player_type;

extern texture_type tux_life,
 tux_right[3],  tux_left[3],
 bigtux_right[3],  bigtux_left[3],
 bigtux_right_jump,  bigtux_left_jump,
 ducktux_right,  ducktux_left,
 skidtux_right,  skidtux_left,
 firetux_right[3],  firetux_left[3],
 bigfiretux_right[3],  bigfiretux_left[3],
 bigfiretux_right_jump,  bigfiretux_left_jump,
 duckfiretux_right,  duckfiretux_left,
 skidfiretux_right,  skidfiretux_left,
 cape_right[2],  cape_left[2],
 bigcape_right[2],  bigcape_left[2];

void player_init(player_type* pplayer);
int player_key_event(player_type* pplayer, SDLKey key, int state);
void player_level_begin(player_type* pplayer);
void player_action(player_type* pplayer);
void player_input(player_type* pplayer);
void player_grabdistros(player_type *pplayer);
void player_draw(player_type* pplayer);
void player_collision(player_type* pplayer,void* p_c_object, int c_object);
void player_kill(player_type *pplayer, int mode);
void player_dying(player_type *pplayer);
void player_remove_powerups(player_type *pplayer);
void player_keep_in_bounds(player_type *pplayer);
int player_on_ground(player_type *pplayer);
int player_under_solid(player_type *pplayer);

#endif /*SUPERTUX_PLAYER_H*/
