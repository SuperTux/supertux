/*
  setup.h
  
  Super Tux - Setup
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/

#ifndef SUPERTUX_SETUP_H
#define SUPERTUX_SETUP_H

#include "menu.h"
#include "sound.h"
#include "type.h"

int faccessible(const char *filename);
int fcreatedir(const char* relative_dir);
int fwriteable(const char *filename);
FILE * opendata(const char * filename, const char * mode);
string_list_type dsubdirs(const char *rel_path, const char* expected_file);
string_list_type dfiles(const char *rel_path, const char* glob, const char* exception_str);
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
void st_abort(const std::string& reason, const std::string& details);
void process_options_menu(void);

/** Return true if the gameloop() was entered, false otherwise */
bool process_load_game_menu();

void update_load_save_game_menu(Menu* pmenu);
void parseargs(int argc, char * argv[]);

#endif /*SUPERTUX_SETUP_H*/

