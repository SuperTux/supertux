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
#include "control/input_manager.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "util/log.hpp"
#include "supertux/globals.hpp"

Config::Config() :
  profile(1),
  display_number(0),
  fullscreen_size(0, 0),
  fullscreen_refresh_rate(0),
  window_size(1280, 800),
  aspect_size(0, 0), // auto detect
  magnification(0.0f),
  use_fullscreen(false),
  video(VideoSystem::AUTO_VIDEO),
  try_vsync(true),
  show_fps(false),
  show_player_pos(false),
  sound_enabled(true),
  music_enabled(true),
  random_seed(0), // set by time(), by default (unless in config)
  start_level(),
  enable_script_debugger(false),
  start_demo(),
  record_demo(),
  tux_spawn_pos(),
  edit_level(),
  locale(),
  keyboard_config(),
  joystick_config(),
  addons(),
  developer_mode(false),
  christmas_mode(false),
  transitions_enabled(true),
  repository_url()
{
}

Config::~Config()
{}

void
Config::load()
{
  auto doc = ReaderDocument::parse("config");
  auto root = doc.get_root();
  if(root.get_name() != "supertux-config")
  {
    throw std::runtime_error("File is not a supertux-config file");
  }

  auto config_lisp = root.get_mapping();
  config_lisp.get("profile", profile);
  config_lisp.get("show_fps", show_fps);
  config_lisp.get("show_player_pos", show_player_pos);
  config_lisp.get("developer", developer_mode);

  if(is_christmas()) {
    if(!config_lisp.get("christmas", christmas_mode))
    {
      christmas_mode = true;
    }
  }
  config_lisp.get("transitions_enabled", transitions_enabled);
  config_lisp.get("locale", locale);
  config_lisp.get("random_seed", random_seed);
  config_lisp.get("repository_url", repository_url);

  ReaderMapping config_video_lisp;
  if(config_lisp.get("video", config_video_lisp))
  {
    config_video_lisp.get("display_number", display_number);
    config_video_lisp.get("fullscreen", use_fullscreen);
    std::string video_string;
    config_video_lisp.get("video", video_string);
    video = VideoSystem::get_video_system(video_string);
    config_video_lisp.get("vsync", try_vsync);

    config_video_lisp.get("fullscreen_width",  fullscreen_size.width);
    config_video_lisp.get("fullscreen_height", fullscreen_size.height);
    config_video_lisp.get("fullscreen_refresh_rate", fullscreen_refresh_rate);

    config_video_lisp.get("window_width",  window_size.width);
    config_video_lisp.get("window_height", window_size.height);

    config_video_lisp.get("aspect_width",  aspect_size.width);
    config_video_lisp.get("aspect_height", aspect_size.height);

    config_video_lisp.get("magnification", magnification);
  }

  ReaderMapping config_audio_lisp;
  if(config_lisp.get("audio", config_audio_lisp)) {
    config_audio_lisp.get("sound_enabled", sound_enabled);
    config_audio_lisp.get("music_enabled", music_enabled);
  }

  ReaderMapping config_control_lisp;
  if (config_lisp.get("control", config_control_lisp))
  {
    ReaderMapping keymap_lisp;
    if (config_control_lisp.get("keymap", keymap_lisp))
    {
      keyboard_config.read(keymap_lisp);
    }

    ReaderMapping joystick_lisp;
    if (config_control_lisp.get("joystick", joystick_lisp))
    {
      joystick_config.read(joystick_lisp);
    }
  }

  ReaderCollection config_addons_lisp;
  if (config_lisp.get("addons", config_addons_lisp))
  {
    for(auto const& addon_node : config_addons_lisp.get_objects())
    {
      if (addon_node.get_name() == "addon")
      {
        auto addon = addon_node.get_mapping();

        std::string id;
        bool enabled = false;
        if (addon.get("id", id) &&
            addon.get("enabled", enabled))
        {
          addons.push_back({id, enabled});
        }
      }
      else
      {
        log_warning << "Unknown token in config file: " << addon_node.get_name() << std::endl;
      }
    }
  }
}

void
Config::save()
{
  Writer writer("config");

  writer.start_list("supertux-config");

  writer.write("profile", profile);
  writer.write("show_fps", show_fps);
  writer.write("show_player_pos", show_player_pos);
  writer.write("developer", developer_mode);
  if(is_christmas()) {
    writer.write("christmas", christmas_mode);
  }
  writer.write("transitions_enabled", transitions_enabled);
  writer.write("locale", locale);
  writer.write("repository_url", repository_url);

  writer.start_list("video");
  {
    writer.write("display_number", display_number);
    writer.write("fullscreen", use_fullscreen);
    writer.write("video", VideoSystem::get_video_string(video));
    writer.write("vsync", try_vsync);

    writer.write("fullscreen_width",  fullscreen_size.width);
    writer.write("fullscreen_height", fullscreen_size.height);
    writer.write("fullscreen_refresh_rate", fullscreen_refresh_rate);

    writer.write("window_width",  window_size.width);
    writer.write("window_height", window_size.height);

    writer.write("aspect_width",  aspect_size.width);
    writer.write("aspect_height", aspect_size.height);

    writer.write("magnification", magnification);
  }
  writer.end_list("video");

  writer.start_list("audio");
  {
    writer.write("sound_enabled", sound_enabled);
    writer.write("music_enabled", music_enabled);
  }
  writer.end_list("audio");

  writer.start_list("control");
  {
    writer.start_list("keymap");
    keyboard_config.write(writer);
    writer.end_list("keymap");

    writer.start_list("joystick");
    joystick_config.write(writer);
    writer.end_list("joystick");
  }
  writer.end_list("control");

  writer.start_list("addons");
  for(const auto& addon : addons)
  {
    writer.start_list("addon");
    writer.write("id", addon.id);
    writer.write("enabled", addon.enabled);
    writer.end_list("addon");
  }
  writer.end_list("addons");

  writer.end_list("supertux-config");
}

/* EOF */
