/*
  setup.h
  
  Super Tux - Setup
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 13, 2000
*/

#ifndef SUPERTUX_SETUP_H
#define SUPERTUX_SETUP_H

#include "menu.h"
#include "sound.h"

int faccessible(char *filename);
int fcreatedir(char* relative_dir);
int fwriteable(char *filename);
char ** dsubdirs(char *rel_path, char* expected_file, int* num);
void free_strings(char **strings, int num);
void st_directory_setup(void);
void st_general_setup(void);
void st_video_setup_sdl(void);
void st_video_setup_gl(void);
void st_video_setup(void);
void st_audio_setup(void);
void st_joystick_setup(void);
void st_shutdown(void);
void st_menu(void);
void st_abort(char * reason, char * details);
void process_options_menu(void);
void process_save_load_game_menu(int save);
void update_load_save_game_menu(menu_type* pmenu, int load);
void parseargs(int argc, char * argv[]);

#endif /*SUPERTUX_SETUP_H*/

