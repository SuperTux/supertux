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

#include <string>
#include <SDL.h>
#include "text.h"
#include "menu.h"
#include "mousecursor.h"

extern std::string datadir;

extern SDL_Surface * screen;
extern text_type black_text;
extern text_type gold_text;
extern text_type white_text;
extern text_type white_small_text;
extern text_type white_big_text;
extern text_type blue_text;
extern text_type red_text;
extern text_type yellow_nums;

extern MouseCursor * mouse_cursor;

extern bool use_gl;
extern bool use_joystick;
extern bool use_fullscreen;
extern bool debug_mode;
extern bool show_fps;

/** The number of the joystick that will be use in the game */
extern int joystick_num;
extern char* level_startup_file;
extern bool launch_worldmap_mode;

/* SuperTux directory ($HOME/.supertux) and save directory($HOME/.supertux/save) */
extern char* st_dir;
extern char* st_save_dir;

extern SDL_Joystick * js;

int wait_for_event(SDL_Event& event,unsigned int min_delay = 0, unsigned int max_delay = 0, bool empty_events = false);

#define JOYSTICK_DEAD_ZONE 4096

#endif /* SUPERTUX_GLOBALS_H */
