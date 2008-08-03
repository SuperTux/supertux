//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_SETUP_H
#define SUPERTUX_SETUP_H

#include "menu.h"
#ifndef NOSOUND
#include "sound.h"
#endif
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
void st_general_free();
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

