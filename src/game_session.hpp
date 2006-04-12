//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Bill Kendrick <bill@newbreedsoftware.com>
//                     Tobias Glaesser <tobi.web@gmx.de>
//                     Ingo Ruhnke <grumbel@gmx.de>
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
#ifndef SUPERTUX_GAMELOOP_H
#define SUPERTUX_GAMELOOP_H

#include <string>
#include <SDL.h>
#include "screen.hpp"
#include "timer.hpp"
#include "statistics.hpp"
#include "math/vector.hpp"
#include "console.hpp"
#include "video/surface.hpp"

/* GameLoop modes */
enum GameSessionMode {
  ST_GL_PLAY,
  ST_GL_TEST,
  ST_GL_LOAD_GAME,
  ST_GL_LOAD_LEVEL_FILE,
  ST_GL_DEMO_GAME
};

enum GameMenuIDs {
  MNID_CONTINUE,
  MNID_ABORTLEVEL
};

extern int game_started;

class Level;
class Sector;
class Statistics;
class DrawingContext;
class CodeController;

/**
 * The GameSession class controlls the controll flow of the Game (the part
 * where you actually play a level)
 */
class GameSession : public Screen, public ConsoleCommandReceiver
{
public:
  GameSession(const std::string& levelfile, GameSessionMode mode,
              Statistics* statistics = NULL);
  ~GameSession();

  void record_demo(const std::string& filename);
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
  void respawn(const std::string& sectorname,
      const std::string& spawnpointname);
  void set_reset_point(const std::string& sectorname,
      const Vector& pos);
  void display_info_box(const std::string& text);
  
  Sector* get_current_sector()
  { return currentsector; }

  Level* get_current_level()
  { return level.get(); }

  void start_sequence(const std::string& sequencename);

  /** returns the "working directory" usually this is the directory where the
   * currently played level resides. This is used when locating additional
   * resources for the current level/world
   */
  std::string get_working_directory();
  bool consoleCommand(std::string command, std::vector<std::string> arguments); /**< callback from Console; return false if command was unknown, true otherwise */

private:
  void restart_level(bool fromBeginning = true);

  void check_end_conditions();
  void process_events();
  void capture_demo_step();

  void levelintro();
  void drawstatus(DrawingContext& context);
  void draw_pause(DrawingContext& context);

  void on_escape_press();
  void process_menu();

  Timer endsequence_timer;
  std::auto_ptr<Level> level;
  std::auto_ptr<Surface> statistics_backdrop;

  Sector* currentsector;

  GameSessionMode mode;
  int levelnb;
  float fps_fps;
  int pause_menu_frame;

  /** If true the end_sequence will be played, user input will be
      ignored while doing that */
  enum EndSequenceState {
    NO_ENDSEQUENCE,
    ENDSEQUENCE_RUNNING, // tux is running right
    ENDSEQUENCE_WAITING  // waiting for the end of the music
  };
  EndSequenceState end_sequence;
  float last_x_pos;
  CodeController* end_sequence_controller;

  bool game_pause;

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
};

#endif /*SUPERTUX_GAMELOOP_H*/

