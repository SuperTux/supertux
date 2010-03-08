//  SuperTux -  A Jump'n Run
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

#include "supertux/gameconfig.hpp"

#include <stdexcept>

#include "addon/addon_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "lisp/writer.hpp"
#include "lisp/parser.hpp"
#include "util/reader.hpp"
#include "supertux/globals.hpp"

Config::Config() :
  profile(1),
  fullscreen_size(800, 600),
  window_size(800, 600),
  aspect_size(0, 0), // auto detect
  magnification(1.0f),
  use_fullscreen(false),
  video(VideoSystem::AUTO_VIDEO),
  try_vsync(true),
  show_fps(false),
  sound_enabled(true),
  music_enabled(true),
  console_enabled(false),
  random_seed(0),          // set by time(), by default (unless in config)
  start_level(),
  enable_script_debugger(false),
  start_demo(),
  record_demo(),
  locale()
{
}

Config::~Config()
{}

void
Config::load()
{
  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse("config");

  const lisp::Lisp* config_lisp = root->get_lisp("supertux-config");
  if(!config_lisp)
    throw std::runtime_error("File is not a supertux-config file");

  config_lisp->get("show_fps", show_fps);
  config_lisp->get("console", console_enabled);
  config_lisp->get("locale", locale);
  config_lisp->get("random_seed", random_seed);

  const lisp::Lisp* config_video_lisp = config_lisp->get_lisp("video");
  if(config_video_lisp) {
    config_video_lisp->get("fullscreen", use_fullscreen);
    std::string video_string;
    config_video_lisp->get("video", video_string);
    video = VideoSystem::get_video_system(video_string);
    config_video_lisp->get("vsync", try_vsync);

    config_video_lisp->get("fullscreen_width",  fullscreen_size.width);
    config_video_lisp->get("fullscreen_height", fullscreen_size.height);

    config_video_lisp->get("window_width",  window_size.width);
    config_video_lisp->get("window_height", window_size.height);

    config_video_lisp->get("aspect_width",  aspect_size.width);
    config_video_lisp->get("aspect_height", aspect_size.height);
  }

  const lisp::Lisp* config_audio_lisp = config_lisp->get_lisp("audio");
  if(config_audio_lisp) {
    config_audio_lisp->get("sound_enabled", sound_enabled);
    config_audio_lisp->get("music_enabled", music_enabled);
  }

  const lisp::Lisp* config_control_lisp = config_lisp->get_lisp("control");
  if(config_control_lisp && g_jk_controller) {
    g_jk_controller->read(*config_control_lisp);
  }

  const lisp::Lisp* config_addons_lisp = config_lisp->get_lisp("addons");
  if(config_addons_lisp) {
    AddonManager::get_instance().read(*config_addons_lisp);
  }
}

void
Config::save()
{
  lisp::Writer writer("config");

  writer.start_list("supertux-config");

  writer.write("show_fps", show_fps);
  writer.write("console", console_enabled);
  writer.write("locale", locale);

  writer.start_list("video");
  writer.write("fullscreen", use_fullscreen);
  writer.write("video", VideoSystem::get_video_string(video));
  writer.write("vsync", try_vsync);

  writer.write("fullscreen_width",  fullscreen_size.width);
  writer.write("fullscreen_height", fullscreen_size.height);

  writer.write("window_width",  window_size.width);
  writer.write("window_height", window_size.height);

  writer.write("aspect_width",  aspect_size.width);
  writer.write("aspect_height", aspect_size.height);

  writer.end_list("video");

  writer.start_list("audio");
  writer.write("sound_enabled", sound_enabled);
  writer.write("music_enabled", music_enabled);
  writer.end_list("audio");

  if(g_jk_controller) {
    writer.start_list("control");
    g_jk_controller->write(writer);
    writer.end_list("control");
  }

  writer.start_list("addons");
  AddonManager::get_instance().write(writer);
  writer.end_list("addons");

  writer.end_list("supertux-config");
}

/* EOF */
