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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_SESSION_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_SESSION_HPP

#include <SDL.h>
#include <squirrel.h>
#include <string>

#include "math/vector.hpp"
#include "object/endsequence.hpp"
#include "supertux/screen.hpp"
#include "video/surface.hpp"

class Level;
class Sector;
class Statistics;
class DrawingContext;
class CodeController;
class Menu;

/**
 * Screen that runs a Level, where Players run and jump through Sectors.
 */
class GameSession : public Screen
{
public:
  GameSession(const std::string& levelfile, Statistics* statistics = NULL);
  ~GameSession();

  void record_demo(const std::string& filename);
  int get_demo_random_seed(const std::string& filename);
  void play_demo(const std::string& filename);

  void draw(DrawingContext& context);
  void update(float frame_ratio);
  void setup();

  void set_current()
  { current_ = this; }
  static GameSession* current()
  { return current_; }

  /// ends the current level
  void finish(bool win = true);
  void respawn(const std::string& sectorname, const std::string& spawnpointname);
  void set_reset_point(const std::string& sectorname, const Vector& pos);
  std::string get_reset_point_sectorname()
  { return reset_sector; }

  Vector get_reset_point_pos()
  { return reset_pos; }

  Sector* get_current_sector()
  { return currentsector; }

  Level* get_current_level()
  { return level.get(); }

  void start_sequence(const std::string& sequencename);

  /**
   * returns the "working directory" usually this is the directory where the
   * currently played level resides. This is used when locating additional
   * resources for the current level/world
   */
  std::string get_working_directory();
  void restart_level();

  void toggle_pause();

  /**
   * Enters or leaves level editor mode
   */
  void set_editmode(bool edit_mode = true);

  /**
   * Forces all Players to enter ghost mode
   */
  void force_ghost_mode();

private:
  void check_end_conditions();
  void process_events();
  void capture_demo_step();

  void drawstatus(DrawingContext& context);
  void draw_pause(DrawingContext& context);

  HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename);
  void on_escape_press();
  void process_menu();

  std::auto_ptr<Level> level;
  std::auto_ptr<Surface> statistics_backdrop;

  // scripts
  typedef std::vector<HSQOBJECT> ScriptList;
  ScriptList scripts;

  Sector* currentsector;

  int levelnb;
  int pause_menu_frame;

  EndSequence* end_sequence;

  bool  game_pause;
  float speed_before_pause;

  std::string levelfile;

  // reset point (the point where tux respawns if he dies)
  std::string reset_sector;
  Vector reset_pos;

  // the sector and spawnpoint we should spawn after this frame
  std::string newsector;
  std::string newspawnpoint;

  static GameSession* current_;

  Statistics* best_level_statistics;

  std::ostream* capture_demo_stream;
  std::string capture_file;
  std::istream* playback_demo_stream;
  CodeController* demo_controller;

  std::auto_ptr<Menu> game_menu;

  float play_time; /**< total time in seconds that this session ran interactively */

  bool edit_mode; /**< true if GameSession runs in level editor mode */
  bool levelintro_shown; /**< true if the LevelIntro screen was already shown */

private:
  GameSession(const GameSession&);
  GameSession& operator=(const GameSession&);
};

#endif /*SUPERTUX_GAMELOOP_H*/
