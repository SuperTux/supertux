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

#include "control/joystick_config.hpp"
#include "control/keyboard_config.hpp"
#include "math/size.hpp"
#include "math/vector.hpp"
#include "video/video_system.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>

class Config
{
public:
  Config();
  ~Config();

  void load();
  void save();

  int profile;

  /** the width/height to be used to display the game in fullscreen */
  Size fullscreen_size;

  /** refresh rate for use in fullscreen, 0 for auto */
  int fullscreen_refresh_rate;

  /** the width/height of the window managers window */
  Size window_size;

  /** the aspect ratio */
  Size aspect_size;

  float magnification;

  bool use_fullscreen;
  VideoSystem::Enum video;
  bool try_vsync;
  bool show_fps;
  bool show_player_pos;
  bool sound_enabled;
  bool music_enabled;

  /** initial random seed.  0 ==> set from time() */
  int random_seed;

  /** this variable is set if supertux should start in a specific level */
  std::string start_level;
  bool enable_script_debugger;
  std::string start_demo;
  std::string record_demo;
  
  /** this variable is set if tux should spawn somewhere which isn't the "main" spawn point*/
  boost::optional<Vector> tux_spawn_pos;

  /** The level that should be launched in the editor*/
  boost::optional<std::string> edit_level;

  /** force SuperTux language to this locale, e.g. "de". A file
      "data/locale/xx.po" must exist for this to work. An empty string
      means autodetect. */
  std::string locale;

  KeyboardConfig keyboard_config;
  JoystickConfig joystick_config;

  struct Addon
  {
    std::string id;
    bool enabled;
  };
  std::vector<Addon> addons;

  bool developer_mode;
  bool christmas_mode;
  bool transitions_enabled;

  std::string repository_url;

  bool is_christmas() const {
    time_t today = time(0);

    struct tm saint_nicholas_day = *localtime(&today);
    saint_nicholas_day.tm_mday = 6;
    saint_nicholas_day.tm_mon = 12; // Actually December not November

    const time_t t = mktime(&saint_nicholas_day);
    return today >= t + localtime(&t)->tm_gmtoff;
  }
};

#endif

/* EOF */
