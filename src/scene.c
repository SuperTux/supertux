//
// C Implementation: scene
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "scene.h"

void set_defaults(void)
{
  int i;

  /* Reset arrays: */

  for (i = 0; i < NUM_BOUNCY_DISTROS; i++)
    bouncy_distros[i].alive = NO;

  for (i = 0; i < NUM_BROKEN_BRICKS; i++)
    broken_bricks[i].alive = NO;

  for (i = 0; i < NUM_BOUNCY_BRICKS; i++)
    bouncy_bricks[i].alive = NO;

  for (i = 0; i < NUM_BAD_GUYS; i++)
  {
    /*bad_guys[i].alive = NO;*/
    badguy_init(&bad_guys[i]);
    }
 
  for (i = 0; i < NUM_FLOATING_SCORES; i++)
    floating_scores[i].alive = NO;

  for (i = 0; i < NUM_UPGRADES; i++)
  {
    /*upgrades[i].alive = NO;*/
    upgrade_init(&upgrades[i]);
    }

  for (i = 0; i < NUM_BULLETS; i++)
  {
    /*bullets[i].alive = NO;*/
    bullet_init(&bullets[i]);
    }


  /* Set defaults: */
  
  scroll_x = 0;

  score_multiplier = 1;
  super_bkgd_time = 0;

  counting_distros = NO;
  distro_counter = 0;

  endpos = 0;

  /* set current song/music */
  current_music = LEVEL_MUSIC;
}

/* Add score: */

void add_score(int x, int y, int s)
{
  int i, found;


  /* Add the score: */

  score = score + s;


  /* Add a floating score thing to the game: */

  found = -1;

  for (i = 0; i < NUM_FLOATING_SCORES && found == -1; i++)
    {
      if (!floating_scores[i].alive)
        found = i;
    }


  if (found != -1)
    {
	floating_score_init(&floating_scores[i],x,y,s);
    }
}

/* Add a bouncy distro: */

void add_bouncy_distro(float x, float y)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BOUNCY_DISTROS && found == -1; i++)
    {
      if (!bouncy_distros[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bouncy_distros[found].alive = YES;
      bouncy_distros[found].x = x;
      bouncy_distros[found].y = y;
      bouncy_distros[found].ym = -6;
    }
}


/* Add broken brick pieces: */

void add_broken_brick(float x, float y)
{
  add_broken_brick_piece(x, y, -4, -16);
  add_broken_brick_piece(x, y + 16, -6, -12);

  add_broken_brick_piece(x + 16, y, 4, -16);
  add_broken_brick_piece(x + 16, y + 16, 6, -12);
}


/* Add a broken brick piece: */

void add_broken_brick_piece(float x, float y, float xm, float ym)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BROKEN_BRICKS && found == -1; i++)
    {
      if (!broken_bricks[i].alive)
        found = i;
    }

  if (found != -1)
    {
      broken_bricks[found].alive = YES;
      broken_bricks[found].x = x;
      broken_bricks[found].y = y;
      broken_bricks[found].xm = xm;
      broken_bricks[found].ym = ym;
    }
}


/* Add a bouncy brick piece: */

void add_bouncy_brick(float x, float y)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BOUNCY_BRICKS && found == -1; i++)
    {
      if (!bouncy_bricks[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bouncy_bricks[found].alive = YES;
      bouncy_bricks[found].x = x;
      bouncy_bricks[found].y = y;
      bouncy_bricks[found].offset = 0;
      bouncy_bricks[found].offset_m = -BOUNCY_BRICK_SPEED;
      bouncy_bricks[found].shape = shape(x, y);
    }
}


/* Add a bad guy: */

void add_bad_guy(float x, float y, int kind)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BAD_GUYS && found == -1; i++)
    {
      if (!bad_guys[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bad_guys[found].alive = YES;
      bad_guys[found].mode = NORMAL;
      bad_guys[found].dying = NO;
      bad_guys[found].kind = kind;
      bad_guys[found].x = x;
      bad_guys[found].y = y;
      bad_guys[found].xm = 1.3;
      bad_guys[found].ym = 1.5;
      bad_guys[found].dir = LEFT;
      bad_guys[found].seen = NO;
      timer_init(&bad_guys[found].timer);
    }
}
