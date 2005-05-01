//  $Id: configfile.h 2293 2005-03-25 20:39:56Z matzebraun $
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Michael George <mike@georgetech.com>
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

class Config {
public:
  Config();
  ~Config();
  
  void load();
  void save();

  /** screen width in pixel (warning: this is the real screen width+height,
   * supertux is using a logical width+height and not this one)
   */
  int screenwidth;
  int screenheight;
  bool use_gl;

  int audio_frequency;
  int audio_channels;
  int audio_voices;
  int audio_chunksize;
  
  bool use_fullscreen;
  bool show_fps;
  bool sound_enabled;
  bool music_enabled;
  bool cheats_enabled;

  /** this variable is set if supertux should start in a specific level */
  std::string start_level;
};

extern Config* config;

#endif
