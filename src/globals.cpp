//
// C Implementation: globals
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "globals.h"

SDL_Surface * screen;
text_type black_text, gold_text, blue_text, red_text, yellow_nums, white_text, white_small_text, white_big_text;

int use_gl, use_joystick, use_fullscreen, debug_mode, show_fps;

/* SuperTux directory ($HOME/.supertux) and save directory($HOME/.supertux/save) */
char *st_dir, *st_save_dir;

#ifdef JOY_YES
SDL_Joystick * js;
#endif


