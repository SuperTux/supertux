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

int score, distros, level, next_level, game_pause, quit, score_multiplier, endpos, counting_distros, distro_counter;
timer_type  super_bkgd_timer;
float scroll_x;
int frame;
bouncy_distro_type *bouncy_distros;
broken_brick_type *broken_bricks;
bouncy_brick_type *bouncy_bricks;
bad_guy_type *bad_guys;
floating_score_type *floating_scores;
upgrade_type *upgrades;
bullet_type *bullets;
int num_bad_guys;
int num_bouncy_distros;
int num_broken_bricks;
int num_bouncy_bricks;
int num_floating_scores;
int num_upgrades;
int num_bullets;
player_type tux;
texture_type img_box_full, img_box_empty, img_mints, img_coffee, img_super_bkgd, img_red_glow;
timer_type time_left;
double frame_ratio;

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
  /* Set defaults: */
  
  scroll_x = 0;

  score_multiplier = 1;
  timer_init(&super_bkgd_timer, YES);

  counting_distros = NO;
  distro_counter = 0;

  endpos = 0;

  /* set current song/music */
  set_current_music(LEVEL_MUSIC);
}

/* Add score: */

void add_score(float x, float y, int s)
{
  int i, found;


  /* Add the score: */

  score += s;


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
  floating_scores = (floating_score_type*) realloc(floating_scores,num_floating_scores*sizeof(floating_score_type));
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
  bouncy_distros = (bouncy_distro_type*) realloc(bouncy_distros,num_bouncy_distros*sizeof(bouncy_distro_type));
  found = num_bouncy_distros - 1;
  }
    
  if (found != -1)
    {
	bouncy_distro_init(&bouncy_distros[found],x,y);
    }
}


/* Add broken brick pieces: */

void add_broken_brick(float x, float y)
{
  add_broken_brick_piece(x, y, -1, -4);
  add_broken_brick_piece(x, y + 16, -1.5, -3);

  add_broken_brick_piece(x + 16, y, 1, -4);
  add_broken_brick_piece(x + 16, y + 16, 1.5, -3);
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
  broken_bricks = (broken_brick_type*) realloc(broken_bricks,num_broken_bricks*sizeof(broken_brick_type));
  found = num_broken_bricks - 1;
  }

  if (found != -1)
    {
       broken_brick_init(&broken_bricks[found], x, y, xm, ym);
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
  bouncy_bricks = (bouncy_brick_type*) realloc(bouncy_bricks,num_bouncy_bricks*sizeof(bouncy_brick_type));
  found = num_bouncy_bricks - 1;
  }

  if (found != -1)
    {
      bouncy_brick_init(&bouncy_bricks[found],x,y);
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
  bad_guys = (bad_guy_type*) realloc(bad_guys,num_bad_guys*sizeof(bad_guy_type));
  found = num_bad_guys - 1;
  }

  if (found != -1)
    {
       badguy_init(&bad_guys[found], x, y, kind);
    }
}

/* Add an upgrade: */

void add_upgrade(float x, float y, int dir, int kind)
{
  int i, found;

  found = -1;

  for (i = 0; i < num_upgrades && found == -1; i++)
    {
      if (!upgrades[i].base.alive)
        found = i;
    }

  if (found == -1)
  {
  ++num_upgrades;
  upgrades = (upgrade_type*) realloc(upgrades,num_upgrades*sizeof(upgrade_type));
  found = num_upgrades - 1;
  }

  if (found != -1)
    {
      upgrade_init(&upgrades[found], x, y, dir, kind);
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
  bullets = (bullet_type*) realloc(bullets,num_bullets*sizeof(bullet_type));
  found = num_bullets - 1;
  }

  if (found != -1)
    {
      bullet_init(&bullets[found], x, y, xm, dir);

      play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);
    }
}

