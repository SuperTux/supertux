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

SDL_Surface * screen;
SDL_Surface * letters_black, * letters_gold, * letters_blue, * letters_red;

int use_joystick, use_fullscreen;

int game_started;

#ifdef JOY_YES
SDL_Joystick * js;
#endif

#endif /* SUPERTUX_GLOBALS_H */
