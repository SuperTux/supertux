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

#include "special/timer.h"
#include "special/base.h"
#include "special/frame_rate.h"

using namespace SuperTux;

/* GameLoop modes */

#define ST_GL_PLAY 0
#define ST_GL_TEST 1
#define ST_GL_LOAD_GAME 2
#define ST_GL_LOAD_LEVEL_FILE  3
#define ST_GL_DEMO_GAME  4

enum GameMenuIDs {
  MNID_CONTINUE,
  MNID_ABORTLEVEL
  };

extern int game_started;

class Level;
class Sector;

namespace SuperTux {
class DrawingContext;
}

/** The GameSession class controlls the controll flow of a World, ie.
    present the menu on specifc keypresses, render and update it while
    keeping the speed and framerate sane, etc. */
class GameSession
{
private:
  Timer fps_timer;
  Timer frame_timer;
  Timer endsequence_timer;
  Level* level;
  Sector* currentsector;

  int st_gl_mode;
  int levelnb;
  float fps_fps;
  FrameRate frame_rate;
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

  bool game_pause;

  std::string levelname;
  bool flip_level;

  // the sector and spawnpoint we shoudl spawn after this frame
  std::string newsector;
  std::string newspawnpoint;
public:
  enum ExitStatus { ES_NONE, ES_LEVEL_FINISHED, ES_GAME_OVER, ES_LEVEL_ABORT };
private:
  ExitStatus exit_status;
public:
  DrawingContext* context;
  Timer time_left;

  GameSession(const std::string& level, int mode, bool flip_level_ = false);
  ~GameSession();

  /** Enter the busy loop */
  ExitStatus run();

  void draw();
  void action(double frame_ratio);

  void set_current()
  { current_ = this; }
  static GameSession* current() { return current_; }

  void respawn(const std::string& sectorname,
      const std::string& spawnpointname);
  Sector* get_current_sector()
  { return currentsector; }
  
private:
  static GameSession* current_;

  // for cheating
  std::string last_keys;
  // for fire works
  Timer random_timer;

  void restart_level();

  void check_end_conditions();
  void start_timers();
  void process_events();

  void levelintro();
  void drawstatus(DrawingContext& context);
  void drawendscreen();
  void drawresultscreen(void);

  void on_escape_press();
  void process_menu();
};

std::string slotinfo(int slot);

bool rectcollision(base_type* one, base_type* two);
void bumpbrick(float x, float y);

/** Return true if the gameloop() was entered, false otherwise */
bool process_load_game_menu();

#endif /*SUPERTUX_GAMELOOP_H*/

