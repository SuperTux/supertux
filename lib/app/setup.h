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
#include <set>
#include <string>
#include "../gui/menu.h"
#include "../special/base.h"

namespace SuperTux {

/// File system utility functions
struct FileSystem
  {
    static int faccessible(const std::string& filename);
    static int fcreatedir(const std::string& relative_dir);
    static int fwriteable(const std::string& filename);
    static std::set<std::string> read_directory(const std::string& pathname);
    static std::set<std::string> dsubdirs(const std::string& rel_path, const std::string& expected_file);
    static std::set<std::string> dfiles(const std::string& rel_path, const std::string& glob, const std::string& exception_str);
  };

/// All you need to get an application up and running
struct Setup
  {
    static void info(const std::string& _package_name, const std::string& _package_symbol_name, const std::string& _package_version);
    static void directories(void);
    static void general(void);
    static void general_free();
    static void video(unsigned int screen_w, unsigned int screen_h);
    static void audio(void);
    static void joystick(void);
    static void parseargs(int argc, char * argv[]);

    private:
    static void video_sdl(unsigned int screen_w, unsigned int screen_h);
    static void video_gl(unsigned int screen_w, unsigned int screen_h);
  };

/// Termination handling
struct Termination
  {
    static void shutdown(void);
    static void abort(const std::string& reason, const std::string& details);
  };
  
} //namespace SuperTux

#endif /*SUPERTUX_SETUP_H*/

