/*
  globals.h
  
  Super Tux - Global Variabls
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/


#ifndef SUPERTUX_GLOBALS_H
#define SUPERTUX_GLOBALS_H

#include <SDL.h>
#include "text.h"
#include "menu.h"

extern SDL_Surface * screen;
extern text_type black_text, gold_text, white_text, white_small_text, white_big_text, blue_text, red_text, yellow_nums;

extern int use_gl, use_joystick, use_fullscreen, debug_mode, show_fps;

/* SuperTux directory ($HOME/.supertux) and save directory($HOME/.supertux/save) */
extern char *st_dir, *st_save_dir;

#ifdef JOY_YES
extern SDL_Joystick * js;
#endif

#endif /* SUPERTUX_GLOBALS_H */
