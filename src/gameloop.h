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

#ifndef NOSOUND
#include "sound.h"
#endif
#include "type.h"
#include "level.h"
#include "world.h"

/* GameLoop modes */

#define ST_GL_PLAY 0
#define ST_GL_TEST 1
#define ST_GL_LOAD_GAME 2
#define ST_GL_LOAD_LEVEL_FILE  3
#define ST_GL_DEMO_GAME  4

extern int game_started;

class World;

/** The GameSession class controlls the controll flow of a World, ie.
    present the menu on specifc keypresses, render and update it while
    keeping the speed and framerate sane, etc. */
class GameSession
{
 private:
  Timer fps_timer;
  Timer frame_timer;
  Timer endsequence_timer;
  World* world;
  int st_gl_mode;
  int levelnb;
  float fps_fps;
  unsigned int last_update_time;
  unsigned int update_time;
  int pause_menu_frame;
  int debug_fps;
#ifdef TSCONTROL
  int old_mouse_y;
#endif

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

  // FIXME: Hack for restarting the level
  std::string subset;

 public:
  enum ExitStatus { ES_NONE, ES_LEVEL_FINISHED, ES_GAME_OVER, ES_LEVEL_ABORT };
 private:
  ExitStatus exit_status;
 public:

  Timer time_left;

  GameSession(const std::string& subset, int levelnb, int mode);
  ~GameSession();

  /** Enter the busy loop */
  ExitStatus run();

  void draw();
  void action(double frame_ratio);

  Level* get_level() { return world->get_level(); }
  World* get_world() { return world; }

  static GameSession* current() { return current_; }
 private:
  static GameSession* current_;

  void restart_level();

  void check_end_conditions();
  void start_timers();
  void process_events();

  void levelintro();
  void drawstatus();
  void drawendscreen();
  void drawresultscreen(void);

 private:
  void on_escape_press();
  void process_menu();
};

std::string slotinfo(int slot);

bool rectcollision(base_type* one, base_type* two);
void bumpbrick(float x, float y);

#endif /*SUPERTUX_GAMELOOP_H*/

