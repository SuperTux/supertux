/*
  globals.h
  
  Super Tux - Global Variabls
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 21, 2000
*/


#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL.h>
#include <SDL_image.h>

#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

SDL_Surface * screen;
SDL_Surface * letters_black, * letters_gold, * letters_blue, * letters_red;

int use_joystick, use_sound, use_fullscreen;

#ifdef JOY_YES
SDL_Joystick * js;
#endif

#endif /* GLOBALS_H */
