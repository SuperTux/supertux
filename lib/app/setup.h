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

#include <vector>
#include <string>
#include "../gui/menu.h"
#include "../audio/sound.h"
#include "../special/base.h"

namespace SuperTux {

int faccessible(const char *filename);
int fcreatedir(const char* relative_dir);
int fwriteable(const char *filename);
std::vector<std::string> read_directory(const std::string& pathname);

FILE * opendata(const char * filename, const char * mode);
string_list_type dsubdirs(const char *rel_path, const char* expected_file);
string_list_type dfiles(const char *rel_path, const char* glob, const char* exception_str);
void free_strings(char **strings, int num);
void st_info_setup(const std::string& _package_name, const std::string& _package_symbol_name, const std::string& _package_version);
void st_directory_setup(void);
void st_general_setup(void);
void st_general_free();
void st_video_setup_sdl(unsigned int screen_w, unsigned int screen_h);
void st_video_setup_gl(unsigned int screen_w, unsigned int screen_h);
void st_video_setup(unsigned int screen_w, unsigned int screen_h);
void st_audio_setup(void);
void st_joystick_setup(void);
void st_shutdown(void);
void st_abort(const std::string& reason, const std::string& details);

void parseargs(int argc, char * argv[]);

}

#endif /*SUPERTUX_SETUP_H*/

