/*
  gameloop.h
  
  Super Tux - Game Loop!
  
  by Bill Kendrick & Tobias Glaesser <tobi.web@gmx.de>
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/

#ifndef SUPERTUX_GAMELOOP_H
#define SUPERTUX_GAMELOOP_H

#include "sound.h"
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
  World* world;
  int st_gl_mode;
  int levelnb;
  float fps_fps;
  unsigned int last_update_time;
  unsigned int update_time;
  int pause_menu_frame;
  int debug_fps;
  bool game_pause;

  // FIXME: Hack for restarting the level
  std::string subset;

 public:
  enum ExitStatus { NONE, LEVEL_FINISHED, GAME_OVER, LEVEL_ABORT };
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
};

std::string slotinfo(int slot);

bool rectcollision(base_type* one, base_type* two);
void bumpbrick(float x, float y);

#endif /*SUPERTUX_GAMELOOP_H*/

