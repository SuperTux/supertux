/*
  globals.h
  
  Super Tux - Global Variabls
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 21, 2000
*/


#ifndef SUPERTUX_GLOBALS_H
#define SUPERTUX_GLOBALS_H

#include <SDL.h>
#include "text.h"

SDL_Surface * screen;
text_type black_text, gold_text, blue_text, red_text;

int use_gl, use_joystick, use_fullscreen, debug_mode, show_fps;

int game_started, level_editor_started;

/* SuperTux directory ($HOME/.supertux) and save directory($HOME/.supertux/save) */
char *st_dir, *st_save_dir;

#ifdef JOY_YES
SDL_Joystick * js;
#endif

#endif /* SUPERTUX_GLOBALS_H */
