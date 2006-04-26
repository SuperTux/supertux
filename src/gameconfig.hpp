//  $Id$
//
//  SuperTux=
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#ifndef SUPERTUX_CONFIG_H
#define SUPERTUX_CONFIG_H

#include <string>

class Config
{
public:
  Config();
  ~Config();
  
  void load();
  void save();

  /** screen width in pixel (warning: this is the real screen width+height,
   * you should use the logical SCREEN_WIDTH and SCREEN_HEIGHT for your
   * rendering code.)
   */
  int screenwidth;
  int screenheight;

  bool use_fullscreen;
  bool show_fps;
  bool sound_enabled;
  bool music_enabled;
  bool cheats_enabled;

  int random_seed;            // initial random seed.  0 ==> set from time()

  /** this variable is set if supertux should start in a specific level */
  std::string start_level;
  bool enable_script_debugger;
  std::string start_demo;
  std::string record_demo;
};

extern Config* config;

#endif
