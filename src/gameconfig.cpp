//  $Id: configfile.cpp 2212 2004-11-28 14:57:45Z matzebraun $
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
#include <config.h>

#include "gameconfig.h"

#include <cstdlib>
#include <string>
#include <stdexcept>
#include <sstream>
#include <fstream>

#include "app/setup.h"
#include "app/globals.h"
#include "audio/sound_manager.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "control/joystickkeyboardcontroller.h"
#include "main.h"

using namespace SuperTux;

Config* config = 0;

Config::Config()
{
  use_fullscreen = true;
  show_fps = false;
  sound_enabled = true;
  music_enabled = true;
  cheats_enabled = false;

  screenwidth = 800;
  screenheight = 600;
  use_gl = true;

  audio_frequency = MIX_DEFAULT_FREQUENCY;
  audio_channels = MIX_DEFAULT_CHANNELS;
  audio_chunksize = 2048;
  audio_voices = MIX_CHANNELS;
}

Config::~Config()
{}

void
Config::load()
{
  lisp::Parser parser;
  std::auto_ptr<lisp::Lisp> root (parser.parse(user_dir + "/config"));

  const lisp::Lisp* config_lisp = root->get_lisp("supertux-config");
  if(!config_lisp)
    throw std::runtime_error("File is not a supertux-config file");

  config_lisp->get("show_fps", show_fps);
  config_lisp->get("cheats", cheats_enabled);

  const lisp::Lisp* config_video_lisp = config_lisp->get_lisp("video");
  if(config_video_lisp) {
    config_video_lisp->get("fullscreen", use_fullscreen);
    config_video_lisp->get("width", screenwidth);
    config_video_lisp->get("height", screenheight);
  }

  const lisp::Lisp* config_audio_lisp = config_lisp->get_lisp("audio");
  if(config_audio_lisp) {
    config_audio_lisp->get("sound_enabled", sound_enabled);
    config_audio_lisp->get("music_enabled", music_enabled);
    config_audio_lisp->get("frequency", audio_frequency);
    config_audio_lisp->get("channels", audio_channels);
    config_audio_lisp->get("voices", audio_voices);
    config_audio_lisp->get("chunksize", audio_chunksize);
  }

  const lisp::Lisp* config_control_lisp = config_lisp->get_lisp("control");
  if(config_control_lisp && main_controller) {
    main_controller->read(*config_control_lisp);
  }
}

void
Config::save()
{
  std::string configfile = user_dir + "/config";
  std::ofstream file( (user_dir + "/config").c_str() );
  if(!file.good()) {
    std::stringstream msg;
    msg << "Couldn't write config file '" << configfile << "'";
    throw std::runtime_error(msg.str());
  }
  lisp::Writer writer(file);

  writer.start_list("supertux-config");

  writer.write_bool("show_fps", show_fps);
  writer.write_bool("cheats", cheats_enabled);

  writer.start_list("video");
  writer.write_bool("fullscreen", use_fullscreen);
  writer.write_int("width", screenwidth);
  writer.write_int("height", screenheight);
  writer.end_list("video");

  writer.start_list("audio");
  writer.write_bool("sound_enabled", sound_enabled);
  writer.write_bool("music_enabled", music_enabled);
  writer.write_int("frequency", audio_frequency);
  writer.write_int("channels", audio_channels);
  writer.write_int("voices", audio_voices);
  writer.write_int("chunksize", audio_chunksize);
  writer.end_list("audio");

  if(main_controller) {
    writer.start_list("control");
    main_controller->write(writer);
    writer.end_list("control");
  }
  
  writer.end_list("supertux-config");
}
