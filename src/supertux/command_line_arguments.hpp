//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_OPTIONS_HPP
#define HEADER_SUPERTUX_SUPERTUX_OPTIONS_HPP

#include <util/optional.hpp>

#include "math/size.hpp"
#include "math/vector.hpp"
#include "util/log.hpp"
#include "video/video_system.hpp"

class Config;

/** Command line argument parsing */
class CommandLineArguments
{
public:
  enum Action
  {
    NO_ACTION,
    PRINT_VERSION,
    PRINT_HELP,
    PRINT_DATADIR
  };

private:
  Action m_action;
  LogLevel m_log_level;

public:
  std::experimental::optional<std::string> datadir;
  std::experimental::optional<std::string> userdir;

  std::experimental::optional<Size> fullscreen_size;
  std::experimental::optional<int> fullscreen_refresh_rate;
  std::experimental::optional<Size> window_size;
  std::experimental::optional<Size> aspect_size;
  

  // std::experimental::optional<float> magnification;

  std::experimental::optional<bool> use_fullscreen;
  std::experimental::optional<VideoSystem::Enum> video;
  // std::experimental::optional<bool> try_vsync;
  std::experimental::optional<bool> show_fps;
  std::experimental::optional<bool> show_player_pos;
  std::experimental::optional<bool> sound_enabled;
  std::experimental::optional<bool> music_enabled;

  // std::experimental::optional<int> random_seed;

  std::experimental::optional<std::string> start_level;
  std::experimental::optional<bool> enable_script_debugger;
  std::experimental::optional<std::string> start_demo;
  std::experimental::optional<std::string> record_demo;
  std::experimental::optional<Vector> tux_spawn_pos;

  std::experimental::optional<bool> developer_mode;
  
  std::experimental::optional<bool> christmas_mode;

  std::experimental::optional<std::string> repository_url;

  std::experimental::optional<std::string> edit_level;

  // std::experimental::optional<std::string> locale;

public:
  CommandLineArguments();
  ~CommandLineArguments();

  Action get_action() const { return m_action; }
  LogLevel get_log_level() const { return m_log_level; }

  void parse_args(int argc, char** argv);

  void print_help(const char* arg0) const;
  void print_version() const;
  void print_datadir() const;

  void merge_into(Config& config);

private:
  CommandLineArguments(const CommandLineArguments&) = delete;
  CommandLineArguments& operator=(const CommandLineArguments&) = delete;
};

#endif

/* EOF */
