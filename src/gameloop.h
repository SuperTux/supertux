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

// FIXME: Make this local to the gamesession
extern Level current_level;

extern int game_started;

class GameSession
{
 private:
    timer_type fps_timer, frame_timer;
 public:
  GameSession(const char * subset, int levelnb, int mode);
  int run();
};

void  activate_bad_guys(Level* plevel);
void  savegame(int slot);
void  loadgame(int slot);
std::string slotinfo(int slot);

bool  rectcollision(base_type* one, base_type* two);
void  drawshape(float x, float y, unsigned int c, Uint8 alpha = 255);
void bumpbrick(float x, float y);

/** Try to grab the coin at the given coordinates */
void trygrabdistro(float x, float y, int bounciness);

/** Try to break the brick at the given coordinates */
void trybreakbrick(float x, float y, bool small);

/** Try to get the content out of a bonus box, thus emptying it */
void tryemptybox(float x, float y, int col_side);

/** Try to bumb a badguy that might we walking above Tux, thus shaking
    the tile which the badguy is walking on an killing him this way */
void trybumpbadguy(float x, float y);

#endif /*SUPERTUX_GAMELOOP_H*/

