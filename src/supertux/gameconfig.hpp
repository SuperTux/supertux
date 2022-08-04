//  SuperTux
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

#include "config.h"

#include "control/joystick_config.hpp"
#include "control/keyboard_config.hpp"
#include "math/size.hpp"
#include "math/vector.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>

class Config final
{
public:
  Config();

  void load();
  void save();

  int profile;

  /** the width/height to be used to display the game in fullscreen */
  Size fullscreen_size;

  /** refresh rate for use in fullscreen, 0 for auto */
  int fullscreen_refresh_rate;

  /** the width/height of the window managers window */
  Size window_size;

  /** Window is resizable */
  bool window_resizable;

  /** the aspect ratio */
  Size aspect_size;

#ifdef __EMSCRIPTEN__
  /** @deprecated Whether to automatically resize the game when the browser is resized */
  bool fit_window;
#endif

  float magnification;

  bool use_fullscreen;
  VideoSystem::Enum video;
  bool try_vsync;
  bool show_fps;
  bool show_player_pos;
  bool show_controller;
  bool sound_enabled;
  bool music_enabled;
  int sound_volume;
  int music_volume;

  /** initial random seed.  0 ==> set from time() */
  int random_seed;

  bool enable_script_debugger;
  std::string start_demo;
  std::string record_demo;

  /** this variable is set if tux should spawn somewhere which isn't the "main" spawn point*/
  boost::optional<Vector> tux_spawn_pos;

  /** force SuperTux language to this locale, e.g. "de". A file
      "data/locale/xx.po" must exist for this to work. An empty string
      means autodetect. */
  std::string locale;

  KeyboardConfig keyboard_config;
  JoystickConfig joystick_config;

  bool mobile_controls;

  struct Addon
  {
    std::string id;
    bool enabled;
  };
  std::vector<Addon> addons;

  bool developer_mode;
  bool christmas_mode;
  bool transitions_enabled;
  bool confirmation_dialog;
  bool pause_on_focusloss;
  bool custom_mouse_cursor;

#ifdef ENABLE_DISCORD
  bool enable_discord;
#endif
  bool hide_editor_levelnames;
  Color menubackcolor;
  Color menufrontcolor;
  Color menuhelpbackcolor;
  Color menuhelpfrontcolor;
  Color labeltextcolor;
  Color activetextcolor;
  Color hlcolor;
  Color editorcolor;
  Color editorhovercolor;
  Color editorgrabcolor;
  float menuroundness;

  int editor_selected_snap_grid_size;
  bool editor_render_grid;
  bool editor_snap_to_grid;
  bool editor_render_background;
  bool editor_render_lighting;
  bool editor_autotile_mode;
  bool editor_autotile_help;
  int editor_autosave_frequency;

  bool multiplayer_auto_manage_players;
  bool multiplayer_multibind;
  bool multiplayer_buzz_controllers;

  std::string repository_url;

  bool disable_sound_positioning;

  bool is_christmas() const {
    try
    {
      using namespace boost::gregorian;
      using namespace boost::posix_time;
      date today = second_clock::local_time().date();
      date saint_nicholas_day(today.year(), Dec, 6);
      return today >= saint_nicholas_day;
    }
    catch(...)
    {
      return false;
    }
  }
};

#endif

/* EOF */
