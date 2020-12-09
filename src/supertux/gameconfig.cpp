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

#include "config.h"

#include "editor/overlay_widget.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "util/log.hpp"

Config::Config() :
  profile(1),
  fullscreen_size(0, 0),
  fullscreen_refresh_rate(0),
  window_size(1280, 800),
  window_resizable(true),
  aspect_size(0, 0), // auto detect
  magnification(0.0f),
  use_fullscreen(false),
  video(VideoSystem::VIDEO_AUTO),
  try_vsync(true),
  show_fps(false),
  show_player_pos(false),
  show_controller(false),
  sound_enabled(true),
  music_enabled(true),
  sound_volume(100),
  music_volume(50),
  random_seed(0), // set by time(), by default (unless in config)
  enable_script_debugger(false),
  start_demo(),
  record_demo(),
  tux_spawn_pos(),
  locale(),
  keyboard_config(),
  joystick_config(),
  addons(),
  developer_mode(false),
  christmas_mode(false),
  transitions_enabled(true),
  confirmation_dialog(false),
  pause_on_focusloss(true),
#ifdef ENABLE_DISCORD
  enable_discord(false),
#endif
  hide_editor_levelnames(false),
  editor_autosave_frequency(5),
  repository_url()
{
}

void
Config::load()
{
  auto doc = ReaderDocument::from_file("config");
  auto root = doc.get_root();
  if (root.get_name() != "supertux-config")
  {
    throw std::runtime_error("File is not a supertux-config file");
  }

  auto config_mapping = root.get_mapping();
  config_mapping.get("profile", profile);
  config_mapping.get("show_fps", show_fps);
  config_mapping.get("show_player_pos", show_player_pos);
  config_mapping.get("show_controller", show_controller);
  config_mapping.get("developer", developer_mode);
  config_mapping.get("confirmation_dialog", confirmation_dialog);
  config_mapping.get("pause_on_focusloss", pause_on_focusloss);

  boost::optional<ReaderMapping> config_integrations_mapping;
  if (config_mapping.get("integrations", config_integrations_mapping))
  {
    config_integrations_mapping->get("hide_editor_levelnames", hide_editor_levelnames);
#ifdef ENABLE_DISCORD
    config_integrations_mapping->get("enable_discord", enable_discord);
#endif
  }

  config_mapping.get("editor_autosave_frequency", editor_autosave_frequency);

  EditorOverlayWidget::autotile_help = !developer_mode;

  if (is_christmas()) {
    if (!config_mapping.get("christmas", christmas_mode))
    {
      christmas_mode = true;
    }
  }
  config_mapping.get("transitions_enabled", transitions_enabled);
  config_mapping.get("locale", locale);
  config_mapping.get("random_seed", random_seed);
  config_mapping.get("repository_url", repository_url);

  boost::optional<ReaderMapping> config_video_mapping;
  if (config_mapping.get("video", config_video_mapping))
  {
    config_video_mapping->get("fullscreen", use_fullscreen);
    std::string video_string;
    config_video_mapping->get("video", video_string);
    video = VideoSystem::get_video_system(video_string);
    config_video_mapping->get("vsync", try_vsync);

    config_video_mapping->get("fullscreen_width",  fullscreen_size.width);
    config_video_mapping->get("fullscreen_height", fullscreen_size.height);
    if (fullscreen_size.width < 0 || fullscreen_size.height < 0)
    {
      // Somehow, an invalid size got entered into the config file,
      // let's use the "auto" setting instead.
      fullscreen_size = Size(0, 0);
    }
    config_video_mapping->get("fullscreen_refresh_rate", fullscreen_refresh_rate);

    config_video_mapping->get("window_width",  window_size.width);
    config_video_mapping->get("window_height", window_size.height);

    config_video_mapping->get("window_resizable", window_resizable);

    config_video_mapping->get("aspect_width",  aspect_size.width);
    config_video_mapping->get("aspect_height", aspect_size.height);

    config_video_mapping->get("magnification", magnification);
  }

  boost::optional<ReaderMapping> config_audio_mapping;
  if (config_mapping.get("audio", config_audio_mapping))
  {
    config_audio_mapping->get("sound_enabled", sound_enabled);
    config_audio_mapping->get("music_enabled", music_enabled);
    config_audio_mapping->get("sound_volume", sound_volume);
    config_audio_mapping->get("music_volume", music_volume);
  }

  boost::optional<ReaderMapping> config_control_mapping;
  if (config_mapping.get("control", config_control_mapping))
  {
    boost::optional<ReaderMapping> keymap_mapping;
    if (config_control_mapping->get("keymap", keymap_mapping))
    {
      keyboard_config.read(*keymap_mapping);
    }

    boost::optional<ReaderMapping> joystick_mapping;
    if (config_control_mapping->get("joystick", joystick_mapping))
    {
      joystick_config.read(*joystick_mapping);
    }
  }

  boost::optional<ReaderCollection> config_addons_mapping;
  if (config_mapping.get("addons", config_addons_mapping))
  {
    for (auto const& addon_node : config_addons_mapping->get_objects())
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
  writer.write("show_controller", show_controller);
  writer.write("developer", developer_mode);
  writer.write("confirmation_dialog", confirmation_dialog);
  writer.write("pause_on_focusloss", pause_on_focusloss);
  
  writer.start_list("integrations");
  {
    writer.write("hide_editor_levelnames", hide_editor_levelnames);
#ifdef ENABLE_DISCORD
    writer.write("enable_discord", enable_discord);
#endif
  }
  writer.end_list("integrations");

  writer.write("editor_autosave_frequency", editor_autosave_frequency);

  if (is_christmas()) {
    writer.write("christmas", christmas_mode);
  }
  writer.write("transitions_enabled", transitions_enabled);
  writer.write("locale", locale);
  writer.write("repository_url", repository_url);

  writer.start_list("video");
  writer.write("fullscreen", use_fullscreen);
  if (video == VideoSystem::VIDEO_NULL) {
    // don't save NULL renderer to config as starting SuperTux without
    // getting a window is rather confusing
  } else {
    writer.write("video", VideoSystem::get_video_string(video));
  }
  writer.write("vsync", try_vsync);

  writer.write("fullscreen_width",  fullscreen_size.width);
  writer.write("fullscreen_height", fullscreen_size.height);
  writer.write("fullscreen_refresh_rate", fullscreen_refresh_rate);

  writer.write("window_width",  window_size.width);
  writer.write("window_height", window_size.height);

  writer.write("window_resizable", window_resizable);

  writer.write("aspect_width",  aspect_size.width);
  writer.write("aspect_height", aspect_size.height);

  writer.write("magnification", magnification);

  writer.end_list("video");

  writer.start_list("audio");
  writer.write("sound_enabled", sound_enabled);
  writer.write("music_enabled", music_enabled);
  writer.write("sound_volume", sound_volume);
  writer.write("music_volume", music_volume);
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
  for (const auto& addon : addons)
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
