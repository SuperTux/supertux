/*
  gameloop.h
  
  Super Tux - Game Loop!
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - Junuary 1st, 2004
*/

#if !defined( SUPERTUX_GAMELOOP_H )
#define SUPERTUX_GAMELOOP_H 1

#include "sound.h"
#include "type.h"

/* Bounciness of distros: */

#define NO_BOUNCE 0
#define BOUNCE 1


/* One-ups... */

#define DISTROS_LIFEUP 100


/* Upgrade types: */

enum {
  UPGRADE_MINTS,
  UPGRADE_COFFEE,
  UPGRADE_HERRING
};

/* Array sizes: */

#define NUM_BOUNCY_DISTROS 8
#define NUM_BROKEN_BRICKS 32
#define NUM_BOUNCY_BRICKS 4
#define NUM_FLOATING_SCORES 6
#define NUM_UPGRADES 2


/* Scores: */

#define SCORE_BRICK 5
#define SCORE_DISTRO 25

/* Function prototypes: */

int gameloop(void);
void savegame(void);
void loadgame(char* filename);
int issolid(float x, float y);
int isbrick(float x, float y);
int isice(float x, float y);
int isfullbox(float x, float y);
int rectcollision(itop_type* one, itop_type* two);
void drawshape(float x, float y, unsigned char c);
unsigned char shape(float x, float y);
void bumpbrick(float x, float y);
void trygrabdistro(float x, float y, int bounciness);
void trybreakbrick(float x, float y);
void tryemptybox(float x, float y);
void trybumpbadguy(float x, float y);
void add_bouncy_distro(float x, float y);
void add_broken_brick(float x, float y);
void add_broken_brick_piece(float x, float y, float xm, float ym);
void add_bouncy_brick(float x, float y);
void add_bad_guy(float x, float y, int kind);
void add_upgrade(float x, float y, int kind);
void add_bullet(float x, float y, float xm, int dir);
#endif

