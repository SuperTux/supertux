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

#include <stdlib.h>
#include "scene.h"

/* Initialize all 'dynamic' arrays */
void arrays_init(void)
{
num_bad_guys = 0;
num_bouncy_distros = 0;
num_broken_bricks = 0;
num_bouncy_bricks = 0;
num_floating_scores = 0;
num_upgrades = 0;
num_bullets = 0;
bad_guys = NULL;
bouncy_distros = NULL;
broken_bricks = NULL;
bouncy_bricks = NULL;
floating_scores = NULL;
upgrades = NULL;
bullets = NULL;
}

/* Free memory of 'dynamic' arrays */
void arrays_free(void)
{
free(bad_guys);
free(bouncy_distros);
free(broken_bricks);
free(bouncy_bricks);
free(floating_scores);
free(upgrades);
free(bullets);
}

void set_defaults(void)
{
  int i;

  /* Reset arrays: */

  for (i = 0; i < num_bouncy_distros; i++)
    bouncy_distros[i].base.alive = NO;

  for (i = 0; i < num_broken_bricks; i++)
    broken_bricks[i].base.alive = NO;

  for (i = 0; i < num_bouncy_bricks; i++)
    bouncy_bricks[i].base.alive = NO;
    	
  for (i = 0; i < num_bad_guys; i++)
  {
    /*bad_guys[i].alive = NO;*/
    badguy_init(&bad_guys[i]);
    }
 
  for (i = 0; i < num_floating_scores; i++)
    floating_scores[i].base.alive = NO;

  for (i = 0; i < num_upgrades; i++)
  {
    /*upgrades[i].alive = NO;*/
    upgrade_init(&upgrades[i]);
    }

  for (i = 0; i < num_bullets; i++)
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

  for (i = 0; i < num_floating_scores && found == -1; i++)
    {
      if (!floating_scores[i].base.alive)
        found = i;
    }
    
  if (found == -1)
  {
  ++num_floating_scores;
  floating_scores = realloc(floating_scores,num_floating_scores*sizeof(floating_score_type));
  floating_score_init(&floating_scores[num_floating_scores-1],x,y,s);
  found = -1;
  }

  if (found != -1)
    {
	floating_score_init(&floating_scores[found],x,y,s);
    }
}

/* Add a bouncy distro: */

void add_bouncy_distro(float x, float y)
{
  int i, found;

  found = -1;

  for (i = 0; i < num_bouncy_distros && found == -1; i++)
    {
      if (!bouncy_distros[i].base.alive)
        found = i;
    }
    
  if (found == -1)
  {
  ++num_bouncy_distros;
  bouncy_distros = realloc(bouncy_distros,num_bouncy_distros*sizeof(bouncy_distro_type));
  found = num_bouncy_distros - 1;
  }
    
  if (found != -1)
    {
      bouncy_distros[found].base.alive = YES;
      bouncy_distros[found].base.x = x;
      bouncy_distros[found].base.y = y;
      bouncy_distros[found].base.ym = -6;
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

  for (i = 0; i < num_broken_bricks && found == -1; i++)
    {
      if (!broken_bricks[i].base.alive)
        found = i;
    }

  if (found == -1)
  {
  ++num_broken_bricks;
  broken_bricks = realloc(broken_bricks,num_broken_bricks*sizeof(broken_brick_type));
  found = num_broken_bricks - 1;
  }

  if (found != -1)
    {
      broken_bricks[found].base.alive = YES;
      broken_bricks[found].base.x = x;
      broken_bricks[found].base.y = y;
      broken_bricks[found].base.xm = xm;
      broken_bricks[found].base.ym = ym;
    }
}


/* Add a bouncy brick piece: */

void add_bouncy_brick(float x, float y)
{
  int i, found;

  found = -1;

  for (i = 0; i < num_bouncy_bricks && found == -1; i++)
    {
      if (!bouncy_bricks[i].base.alive)
        found = i;
    }
    
  if (found == -1)
  {
  ++num_bouncy_bricks;
  bouncy_bricks = realloc(bouncy_bricks,num_bouncy_bricks*sizeof(bouncy_brick_type));
  found = num_bouncy_bricks - 1;
  }

  if (found != -1)
    {
      bouncy_bricks[found].base.alive = YES;
      bouncy_bricks[found].base.x = x;
      bouncy_bricks[found].base.y = y;
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

  for (i = 0; i < num_bad_guys && found == -1; i++)
    {
      if (!bad_guys[i].base.alive)
        found = i;
    }
 
  if (found == -1)
  {
  ++num_bad_guys;
  bad_guys = realloc(bad_guys,num_bad_guys*sizeof(bad_guy_type));
  badguy_init(&bad_guys[num_bad_guys-1]);
  found = num_bad_guys - 1;
  }

  if (found != -1)
    {
      bad_guys[found].base.alive = YES;
      bad_guys[found].mode = NORMAL;
      bad_guys[found].dying = NO;
      bad_guys[found].kind = kind;
      bad_guys[found].base.x = x;
      bad_guys[found].base.y = y;
      bad_guys[found].base.xm = 1.3;
      bad_guys[found].base.ym = 1.5;
      bad_guys[found].dir = LEFT;
      bad_guys[found].seen = NO;
      timer_init(&bad_guys[found].timer);
    }
}

/* Add an upgrade: */

void add_upgrade(float x, float y, int kind)
{
  int i, r, found;
  /* we use this pointer to check, if realloc() returned a new memory address */
  upgrade_type * pointee = upgrades;

  found = -1;
  r = 0;

  for (i = 0; i < num_upgrades && found == -1; i++)
    {
      if (!upgrades[i].base.alive)
        found = i;
    }

  if (found == -1)
  {
  ++num_upgrades;
  upgrades = realloc(upgrades,num_upgrades*sizeof(upgrade_type));
  if(upgrades != pointee)
  r = 1;
  upgrade_init(&upgrades[num_upgrades-1]);
  found = num_upgrades - 1;
  }

  if (found != -1)
    {
      if(r == 1)
      {
        for (i = 0; i < num_upgrades && found == -1; i++)
    {
       upgrade_init(&upgrades[i]);
    }
      }
      upgrade_init(&upgrades[found]);
      upgrades[found].base.alive = YES;
      upgrades[found].kind = kind;
      upgrades[found].base.x = x;
      upgrades[found].base.y = y;
      upgrades[found].base.xm = 2;
      upgrades[found].base.ym = -2;
      upgrades[found].base.height = 0;
    }
}

/* Add a bullet: */

void add_bullet(float x, float y, float xm, int dir)
{
  int i, found;
  
  found = -1;

  for (i = 0; i < num_bullets && found == -1; i++)
    {
      if (!bullets[i].base.alive)
        found = i;
    }

  if (found == -1)
  {
  ++num_bullets;
  bullets = realloc(bullets,num_bullets*sizeof(bullet_type));
  bullet_init(&bullets[num_bullets-1]);
  found = num_bullets - 1;
  }

  if (found != -1)
    {
      bullet_init(&bullets[found]);
      bullets[found].base.alive = YES;

      if (dir == RIGHT)
        {
          bullets[found].base.x = x + 32;
          bullets[found].base.xm = BULLET_XM + xm;
        }
      else
        {
          bullets[found].base.x = x;
          bullets[found].base.xm = -BULLET_XM + xm;
        }

      bullets[found].base.y = y;
      bullets[found].base.ym = BULLET_STARTING_YM;

      play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);
    }
}

