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

/* GameLoop modes */

#define ST_GL_PLAY 0
#define ST_GL_TEST 1
#define ST_GL_LOAD_GAME 2
#define ST_GL_LOAD_LEVEL_FILE  3

extern int game_started;

/** The GameSession class controlls the controll flow of a World, ie.
    present the menu on specifc keypresses, render and update it while
    keeping the speed and framerate sane, etc. */
class GameSession
{
 private:
  timer_type fps_timer, frame_timer;
  Level current_level;

 public:
  GameSession();
  GameSession(const std::string& filename);
  GameSession(const std::string& subset, int levelnb, int mode);
  int  run();
  void draw();
  int  action();
  void process_events();

  Level* get_level() { return &current_level; }

  void  savegame(int slot);
  void  loadgame(int slot);

  static GameSession* current() { return current_; }
 private:
  static GameSession* current_;

  void levelintro();
  void start_timers();
  void activate_particle_systems();
};

void  activate_bad_guys(Level* plevel);

std::string slotinfo(int slot);

bool  rectcollision(base_type* one, base_type* two);
void  drawshape(float x, float y, unsigned int c, Uint8 alpha = 255);
void bumpbrick(float x, float y);

#endif /*SUPERTUX_GAMELOOP_H*/

