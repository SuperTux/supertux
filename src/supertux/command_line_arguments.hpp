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

#ifndef HEADER_SUPERTUX_SUPERTUX_COMMAND_LINE_ARGUMENTS_HPP
#define HEADER_SUPERTUX_SUPERTUX_COMMAND_LINE_ARGUMENTS_HPP

#include <boost/optional.hpp>
#include <vector>

#include "math/size.hpp"
#include "math/vector.hpp"
#include "util/log.hpp"
#include "video/video_system.hpp"

class Config;

/** Command line argument parsing */
class CommandLineArguments final
{
public:
  enum Action
  {
    NO_ACTION,
    PRINT_VERSION,
    PRINT_HELP,
    PRINT_DATADIR,
    PRINT_ACKNOWLEDGEMENTS
  };

private:
  Action m_action;
  LogLevel m_log_level;

public:
  boost::optional<std::string> datadir;
  boost::optional<std::string> userdir;

  boost::optional<Size> fullscreen_size;
  boost::optional<int> fullscreen_refresh_rate;
  boost::optional<Size> window_size;
  boost::optional<Size> aspect_size;

  // boost::optional<float> magnification;

  boost::optional<bool> use_fullscreen;
  boost::optional<VideoSystem::Enum> video;
  // boost::optional<bool> try_vsync;
  boost::optional<bool> show_fps;
  boost::optional<bool> show_player_pos;
  boost::optional<bool> sound_enabled;
  boost::optional<bool> music_enabled;

  // boost::optional<int> random_seed;

  std::vector<std::string> filenames;
  boost::optional<bool> enable_script_debugger;
  boost::optional<std::string> start_demo;
  boost::optional<std::string> record_demo;
  boost::optional<Vector> tux_spawn_pos;
  boost::optional<std::string> sector;
  boost::optional<std::string> spawnpoint;

  boost::optional<bool> developer_mode;

  boost::optional<bool> christmas_mode;

  boost::optional<std::string> repository_url;

  boost::optional<bool> editor;
  boost::optional<bool> resave;
  boost::optional<bool> use_local_path;
  // boost::optional<std::string> locale;

public:
  CommandLineArguments();

  Action get_action() const { return m_action; }
  LogLevel get_log_level() const { return m_log_level; }

  void parse_args(int argc, char** argv);

  void print_help(const char* arg0) const;
  void print_version() const;
  void print_datadir() const;
  void print_acknowledgements() const;

  void merge_into(Config& config);

private:
  CommandLineArguments(const CommandLineArguments&) = delete;
  CommandLineArguments& operator=(const CommandLineArguments&) = delete;
};

#endif

/* EOF */
