//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "scene.h"
#include "screen.h"
#include "defines.h"
#include "world.h"
#include "level.h"
#include "tile.h"
#include "resources.h"

texture_type img_distro[4];

World* World::current_ = 0;

World::World()
{
  // FIXME: Move this to action and draw and everywhere else where the
  // world calls child functions
  current_ = this;

  level = new Level;
  tux.init();
}

World::~World()
{
  delete level;
}

void
World::set_defaults()
{
  // Set defaults: 
  scroll_x = 0;

  player_status.score_multiplier = 1;

  counting_distros = false;
  distro_counter = 0;

  /* set current song/music */
  set_current_music(LEVEL_MUSIC);
}

int
World::load(const std::string& subset, int level_nr)
{
  return level->load(subset, level_nr);
}

int
World::load(const std::string& filename)
{
  return level->load(filename);
}

void
World::arrays_free(void)
{
  bad_guys.clear();
  bouncy_distros.clear();
  broken_bricks.clear();
  bouncy_bricks.clear();
  floating_scores.clear();
  upgrades.clear();
  bullets.clear();
  std::vector<ParticleSystem*>::iterator i;
  for(i = particle_systems.begin(); i != particle_systems.end(); ++i) {
    delete *i;
  }
  particle_systems.clear();
}

void
World::activate_bad_guys()
{
  for (std::vector<BadGuyData>::iterator i = level->badguy_data.begin();
       i != level->badguy_data.end();
       ++i)
    {
      add_bad_guy(i->x, i->y, i->kind);
    }
}

void
World::activate_particle_systems()
{
  if (level->particle_system == "clouds")
    {
      particle_systems.push_back(new CloudParticleSystem);
    }
  else if (level->particle_system == "snow")
    {
      particle_systems.push_back(new SnowParticleSystem);
    }
  else if (level->particle_system != "")
    {
      st_abort("unknown particle system specified in level", "");
    }
}

void
World::draw()
{
  int y,x;

  /* Draw the real background */
  if(get_level()->bkgd_image[0] != '\0')
    {
      int s = (int)scroll_x / 30;
      texture_draw_part(&level->img_bkgd, s, 0,0,0,level->img_bkgd.w - s, level->img_bkgd.h);
      texture_draw_part(&level->img_bkgd, 0, 0,screen->w - s ,0,s,level->img_bkgd.h);
    }
  else
    {
      drawgradient(level->bkgd_top_red, level->bkgd_top_green, level->bkgd_top_blue,
                     level->bkgd_bottom_red, level->bkgd_bottom_green, level->bkgd_bottom_blue);
    }
    
  /* Draw particle systems (background) */
  std::vector<ParticleSystem*>::iterator p;
  for(p = particle_systems.begin(); p != particle_systems.end(); ++p)
    {
      (*p)->draw(scroll_x, 0, 0);
    }

  /* Draw background: */
  for (y = 0; y < 15; ++y)
    {
      for (x = 0; x < 21; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32,
                     level->bg_tiles[(int)y][(int)x + (int)(scroll_x / 32)]);
        }
    }

  /* Draw interactive tiles: */
  for (y = 0; y < 15; ++y)
    {
      for (x = 0; x < 21; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32,
                     level->ia_tiles[(int)y][(int)x + (int)(scroll_x / 32)]);
        }
    }

  /* (Bouncy bricks): */
  for (unsigned int i = 0; i < bouncy_bricks.size(); ++i)
    bouncy_bricks[i].draw();

  for (unsigned int i = 0; i < bad_guys.size(); ++i)
    bad_guys[i].draw();

  tux.draw();

  for (unsigned int i = 0; i < bullets.size(); ++i)
    bullets[i].draw();

  for (unsigned int i = 0; i < floating_scores.size(); ++i)
    floating_scores[i].draw();

  for (unsigned int i = 0; i < upgrades.size(); ++i)
    upgrades[i].draw();

  for (unsigned int i = 0; i < bouncy_distros.size(); ++i)
    bouncy_distros[i].draw();

  for (unsigned int i = 0; i < broken_bricks.size(); ++i)
    broken_bricks[i].draw();

  /* Draw foreground: */
  for (y = 0; y < 15; ++y)
    {
      for (x = 0; x < 21; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32,
                     level->fg_tiles[(int)y][(int)x + (int)(scroll_x / 32)]);
        }
    }

  /* Draw particle systems (foreground) */
  for(p = particle_systems.begin(); p != particle_systems.end(); ++p)
    {
      (*p)->draw(scroll_x, 0, 1);
    }
}

void
World::action(double frame_ratio)
{
  /* Handle bouncy distros: */
  for (unsigned int i = 0; i < bouncy_distros.size(); i++)
    bouncy_distros[i].action(frame_ratio);

  /* Handle broken bricks: */
  for (unsigned int i = 0; i < broken_bricks.size(); i++)
    broken_bricks[i].action(frame_ratio);

  /* Handle distro counting: */
  if (counting_distros)
    {
      distro_counter--;

      if (distro_counter <= 0)
        counting_distros = -1;
    }

  // Handle all kinds of game objects
  for (unsigned int i = 0; i < bouncy_bricks.size(); i++)
    bouncy_bricks[i].action(frame_ratio);
  
  for (unsigned int i = 0; i < floating_scores.size(); i++)
    floating_scores[i].action(frame_ratio);

  for (unsigned int i = 0; i < bullets.size(); ++i)
    bullets[i].action(frame_ratio);
  
  for (unsigned int i = 0; i < upgrades.size(); i++)
    upgrades[i].action(frame_ratio);

  for (unsigned int i = 0; i < bad_guys.size(); i++)
    bad_guys[i].action(frame_ratio);

  /* update particle systems */
  std::vector<ParticleSystem*>::iterator p;
  for(p = particle_systems.begin(); p != particle_systems.end(); ++p)
    {
      (*p)->simulate(frame_ratio);
    }

  /* Handle all possible collisions. */
  collision_handler();
}


void
World::collision_handler()
{
  // CO_BULLET & CO_BADGUY check
  for(unsigned int i = 0; i < bullets.size(); ++i)
    {
      for(unsigned int j = 0; j < bad_guys.size(); ++j)
        {
          if(bad_guys[j].dying != DYING_NOT)
            continue;
          if(rectcollision(&bullets[i].base, &bad_guys[j].base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              // collide with bad_guy first, since bullet_collision will
              // delete the bullet
              bad_guys[j].collision(0, CO_BULLET);
              bullets[i].collision(CO_BADGUY);
              break; // bullet is invalid now, so break
            }
        }
    }

  /* CO_BADGUY & CO_BADGUY check */
  for(unsigned int i = 0; i < bad_guys.size(); ++i)
    {
      if(bad_guys[i].dying != DYING_NOT)
        continue;
      
      for(unsigned int j = i+1; j < bad_guys.size(); ++j)
        {
          if(j == i || bad_guys[j].dying != DYING_NOT)
            continue;

          if(rectcollision(&bad_guys[i].base, &bad_guys[j].base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              bad_guys[j].collision(&bad_guys[i], CO_BADGUY);
              bad_guys[i].collision(&bad_guys[j], CO_BADGUY);
            }
        }
    }

  if(tux.dying != DYING_NOT) return;
    
  // CO_BADGUY & CO_PLAYER check 
  for(unsigned int i = 0; i < bad_guys.size(); ++i)
    {
      if(bad_guys[i].dying != DYING_NOT)
        continue;
      
      if(rectcollision_offset(&bad_guys[i].base,&tux.base,0,0))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          if (tux.previous_base.y < tux.base.y &&
              tux.previous_base.y + tux.previous_base.height 
              < bad_guys[i].base.y + bad_guys[i].base.height/2)
            {
              bad_guys[i].collision(&tux, CO_PLAYER, COLLISION_SQUISH);
            }
          else
            {
              tux.collision(&bad_guys[i], CO_BADGUY);
            }
        }
    }

  // CO_UPGRADE & CO_PLAYER check
  for(unsigned int i = 0; i < upgrades.size(); ++i)
    {
      if(rectcollision(&upgrades[i].base, &tux.base))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          upgrades[i].collision(&tux, CO_PLAYER);
        }
    }
}

void
World::add_score(float x, float y, int s)
{
  player_status.score += s;

  FloatingScore new_floating_score;
  new_floating_score.init(x,y,s);
  floating_scores.push_back(new_floating_score);
}

void
World::add_bouncy_distro(float x, float y)
{
  BouncyDistro new_bouncy_distro;
  new_bouncy_distro.init(x,y);
  bouncy_distros.push_back(new_bouncy_distro);
}

void
World::add_broken_brick(Tile* tile, float x, float y)
{
  add_broken_brick_piece(tile, x, y, -1, -4);
  add_broken_brick_piece(tile, x, y + 16, -1.5, -3);

  add_broken_brick_piece(tile, x + 16, y, 1, -4);
  add_broken_brick_piece(tile, x + 16, y + 16, 1.5, -3);
}

void
World::add_broken_brick_piece(Tile* tile, float x, float y, float xm, float ym)
{
  BrokenBrick new_broken_brick;
  new_broken_brick.init(tile, x, y, xm, ym);
  broken_bricks.push_back(new_broken_brick);
}

void
World::add_bouncy_brick(float x, float y)
{
  BouncyBrick new_bouncy_brick;
  new_bouncy_brick.init(x,y);
  bouncy_bricks.push_back(new_bouncy_brick);
}

void
World::add_bad_guy(float x, float y, BadGuyKind kind)
{
  bad_guys.push_back(BadGuy());
  BadGuy& new_bad_guy = bad_guys.back();
  
  new_bad_guy.init(x,y,kind);
}

void
World::add_upgrade(float x, float y, int dir, int kind)
{
  Upgrade new_upgrade;
  new_upgrade.init(x,y,dir,kind);
  upgrades.push_back(new_upgrade);
}

void 
World::add_bullet(float x, float y, float xm, int dir)
{
  Bullet new_bullet;
  new_bullet.init(x,y,xm,dir);
  bullets.push_back(new_bullet);
  
  play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);
}

/* Break a brick: */
void
World::trybreakbrick(float x, float y, bool small)
{
  Level* plevel = get_level();
  
  Tile* tile = gettile(x, y);
  if (tile->brick)
    {
      if (tile->data > 0)
        {
          /* Get a distro from it: */
          add_bouncy_distro(((int)(x + 1) / 32) * 32,
                                  (int)(y / 32) * 32);

          if (!counting_distros)
            {
              counting_distros = true;
              distro_counter = 50;
            }

          if (distro_counter <= 0)
            plevel->change(x, y, TM_IA, tile->next_tile);

          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
          player_status.score = player_status.score + SCORE_DISTRO;
          player_status.distros++;
        }
      else if (!small)
        {
          /* Get rid of it: */
          plevel->change(x, y, TM_IA, tile->next_tile);
          
          /* Replace it with broken bits: */
          add_broken_brick(tile, 
                                 ((int)(x + 1) / 32) * 32,
                                 (int)(y / 32) * 32);
          
          /* Get some score: */
          play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
          player_status.score = player_status.score + SCORE_BRICK;
        }
    }
}

/* Empty a box: */
void
World::tryemptybox(float x, float y, int col_side)
{
  Tile* tile = gettile(x,y);
  if (!tile->fullbox)
    return;

  // according to the collision side, set the upgrade direction
  if(col_side == LEFT)
    col_side = RIGHT;
  else
    col_side = LEFT;

  int posx = ((int)(x+1) / 32) * 32;
  int posy = (int)(y/32) * 32 - 32;
  switch(tile->data)
    {
    case 1: // Box with a distro!
      add_bouncy_distro(posx, posy);
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
      player_status.score = player_status.score + SCORE_DISTRO;
      player_status.distros++;
      break;

    case 2: // Add an upgrade!
      if (tux.size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(posx, posy, col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add an iceflower: */
        add_upgrade(posx, posy, col_side, UPGRADE_ICEFLOWER);
      play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
      break;

    case 3: // Add a golden herring
      add_upgrade(posx, posy, col_side, UPGRADE_HERRING);
      break;

    case 4: // Add a 1up extra
      add_upgrade(posx, posy, col_side, UPGRADE_1UP);
      break;
    default:
      break;
    }

  /* Empty the box: */
  level->change(x, y, TM_IA, tile->next_tile);
}

/* Try to grab a distro: */
void
World::trygrabdistro(float x, float y, int bounciness)
{
  Tile* tile = gettile(x, y);
  if (tile && tile->distro)
    {
      level->change(x, y, TM_IA, tile->next_tile);
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);

      if (bounciness == BOUNCE)
        {
          add_bouncy_distro(((int)(x + 1) / 32) * 32,
                                  (int)(y / 32) * 32);
        }

      player_status.score = player_status.score + SCORE_DISTRO;
      player_status.distros++;
    }
}

/* Try to bump a bad guy from below: */
void
World::trybumpbadguy(float x, float y)
{
  /* Bad guys: */
  for (unsigned int i = 0; i < bad_guys.size(); i++)
    {
      if (bad_guys[i].base.x >= x - 32 && bad_guys[i].base.x <= x + 32 &&
          bad_guys[i].base.y >= y - 16 && bad_guys[i].base.y <= y + 16)
        {
          bad_guys[i].collision(&tux, CO_PLAYER, COLLISION_BUMP);
        }
    }


  /* Upgrades: */
  for (unsigned int i = 0; i < upgrades.size(); i++)
    {
      if (upgrades[i].base.height == 32 &&
          upgrades[i].base.x >= x - 32 && upgrades[i].base.x <= x + 32 &&
          upgrades[i].base.y >= y - 16 && upgrades[i].base.y <= y + 16)
        {
          upgrades[i].base.xm = -upgrades[i].base.xm;
          upgrades[i].base.ym = -8;
          play_sound(sounds[SND_BUMP_UPGRADE], SOUND_CENTER_SPEAKER);
        }
    }
}

/* EOF */

