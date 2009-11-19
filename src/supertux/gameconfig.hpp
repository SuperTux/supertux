//  SuperTux=
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_SUPERTUX_GAMECONFIG_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAMECONFIG_HPP

#include "video/video_systems.hpp"

class Config
{
public:
  Config();
  ~Config();

  void load();
  void save();

  int profile;

  // the width/height to be used to display the game in fullscreen
  int fullscreen_width;
  int fullscreen_height;

  // the width/height of the window managers window 
  int window_width;
  int window_height;

  // the aspect ratio
  int aspect_width;
  int aspect_height;
  
  float magnification;

  bool use_fullscreen;
  VideoSystem video;
  bool try_vsync;
  bool show_fps;
  bool sound_enabled;
  bool music_enabled;
  bool console_enabled;

  int random_seed;            // initial random seed.  0 ==> set from time()

  /** this variable is set if supertux should start in a specific level */
  std::string start_level;
  bool enable_script_debugger;
  std::string start_demo;
  std::string record_demo;

  std::string locale; /**< force SuperTux language to this locale, e.g. "de". A file "data/locale/xx.po" must exist for this to work. An empty string means autodetect. */
};

#endif

/* EOF */
