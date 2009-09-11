//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <iostream>
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

Surface* img_distro[4];

World* World::current_ = 0;

World::World(const std::string& filename)
{
  // FIXME: Move this to action and draw and everywhere else where the
  // world calls child functions
  current_ = this;

  level = new Level(filename);
  tux.init();

  set_defaults();

  get_level()->load_gfx();
  activate_bad_guys();
  activate_particle_systems();
#ifndef NOSOUND
  get_level()->load_song();
#endif

  apply_bonuses();

  scrolling_timer.init(true);
}

World::World(const std::string& subset, int level_nr)
{
  // FIXME: Move this to action and draw and everywhere else where the
  // world calls child functions
  current_ = this;

  level = new Level(subset, level_nr);
  tux.init();

  set_defaults();

  get_level()->load_gfx();
  activate_bad_guys();
  activate_particle_systems();
#ifndef NOSOUND
  get_level()->load_song();
#endif

  apply_bonuses();

  scrolling_timer.init(true);
}

void
World::apply_bonuses()
{
  // Apply bonuses from former levels
  switch (player_status.bonus)
    {
    case PlayerStatus::NO_BONUS:
      break;

    case PlayerStatus::FLOWER_BONUS:
      tux.got_coffee = true;
      // fall through

    case PlayerStatus::GROWUP_BONUS:
      // FIXME: Move this to Player class
      tux.size = BIG;
      tux.base.height = 64;
      tux.base.y -= 32;
      break;
    }
}

World::~World()
{
  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    delete *i;

  for (ParticleSystems::iterator i = particle_systems.begin();
          i != particle_systems.end(); ++i)
    delete *i;

  for (std::vector<BouncyDistro*>::iterator i = bouncy_distros.begin();
       i != bouncy_distros.end(); ++i)
    delete *i;
  
  for (std::vector<BrokenBrick*>::iterator i = broken_bricks.begin();
       i != broken_bricks.end(); ++i)
    delete *i;
  
  for (std::vector<BouncyBrick*>::iterator i = bouncy_bricks.begin();
       i != bouncy_bricks.end(); ++i)
    delete *i;

  for (std::vector<FloatingScore*>::iterator i = floating_scores.begin();
       i != floating_scores.end(); ++i)
    delete *i;
  
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

#ifndef NOSOUND
  /* set current song/music */
  currentmusic = LEVEL_MUSIC;
#endif
}

void
World::activate_bad_guys()
{
  for (std::vector<BadGuyData>::iterator i = level->badguy_data.begin();
       i != level->badguy_data.end();
       ++i)
    {
      add_bad_guy(i->x, i->y, i->kind, i->stay_on_platform);
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
  if(level->img_bkgd)
    {
      int s = (int)((float)scroll_x * ((float)level->bkgd_speed/100.0f)) % screen->w;
      level->img_bkgd->draw_part(s, 0,0,0,level->img_bkgd->w - s, level->img_bkgd->h);
      level->img_bkgd->draw_part(0, 0,screen->w - s ,0,s,level->img_bkgd->h);
    }
  else
    {
      drawgradient(level->bkgd_top, level->bkgd_bottom);
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
    bouncy_bricks[i]->draw();

  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    (*i)->draw();

  tux.draw();

  for (unsigned int i = 0; i < bullets.size(); ++i)
    bullets[i].draw();

  for (unsigned int i = 0; i < floating_scores.size(); ++i)
    floating_scores[i]->draw();

  for (unsigned int i = 0; i < upgrades.size(); ++i)
    upgrades[i].draw();

  for (unsigned int i = 0; i < bouncy_distros.size(); ++i)
    bouncy_distros[i]->draw();

  for (unsigned int i = 0; i < broken_bricks.size(); ++i)
    broken_bricks[i]->draw();

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
#ifndef NOSOUND
#ifdef GP2X
    updateSound();
#endif
#endif
}

void
World::action(double frame_ratio)
{
  tux.action(frame_ratio);
  tux.check_bounds(level->back_scrolling, (bool)level->hor_autoscroll_speed);
  scrolling(frame_ratio);

  /* Handle bouncy distros: */
  for (unsigned int i = 0; i < bouncy_distros.size(); i++)
    bouncy_distros[i]->action(frame_ratio);

  /* Handle broken bricks: */
  for (unsigned int i = 0; i < broken_bricks.size(); i++)
    broken_bricks[i]->action(frame_ratio);

  // Handle all kinds of game objects
  for (unsigned int i = 0; i < bouncy_bricks.size(); i++)
    bouncy_bricks[i]->action(frame_ratio);
  
  for (unsigned int i = 0; i < floating_scores.size(); i++)
    floating_scores[i]->action(frame_ratio);

  for (unsigned int i = 0; i < bullets.size(); ++i)
    bullets[i].action(frame_ratio);
  
  for (unsigned int i = 0; i < upgrades.size(); i++)
    upgrades[i].action(frame_ratio);

  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    (*i)->action(frame_ratio);

  /* update particle systems */
  std::vector<ParticleSystem*>::iterator p;
  for(p = particle_systems.begin(); p != particle_systems.end(); ++p)
    {
      (*p)->simulate(frame_ratio);
    }

  /* Handle all possible collisions. */
  collision_handler();
  
  // Cleanup marked badguys
  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end();
      /* ++i handled at end of the loop */) {
    if ((*i)->is_removable()) {
      delete *i;
      i =  bad_guys.erase(i);
    } else {
      ++i;
    }
  }
}

// the space that it takes for the screen to start scrolling, regarding
// screen bounds (in pixels)
#ifndef RES320X240
#define X_SPACE (400-16)
#else
#define X_SPACE (80-16)
#endif

// the time it takes to move the camera (in ms)
#define CHANGE_DIR_SCROLL_SPEED 2000

/* This functions takes cares of the scrolling */
void World::scrolling(double frame_ratio)
{
  if(level->hor_autoscroll_speed)
    {
    scroll_x += level->hor_autoscroll_speed * frame_ratio;
    return;
    }

  int tux_pos_x = (int)(tux.base.x + (tux.base.width/2));

  if (level->back_scrolling || debug_mode)
  {
    if(tux.old_dir != tux.dir && level->back_scrolling)
      scrolling_timer.start(CHANGE_DIR_SCROLL_SPEED);

    if(scrolling_timer.check())
    {
      float final_scroll_x;
      if (tux.physic.get_velocity_x() > 0)
        final_scroll_x = tux_pos_x - (screen->w - X_SPACE);
      else if (tux.physic.get_velocity_x() < 0)
        final_scroll_x = tux_pos_x - X_SPACE;
      else
      {
        if (tux.dir == RIGHT)
          final_scroll_x = tux_pos_x - (screen->w - X_SPACE);
        else if (tux.dir == LEFT && level->back_scrolling)
          final_scroll_x = tux_pos_x - X_SPACE;
      }

      scroll_x +=   (final_scroll_x - scroll_x)
                  / (frame_ratio * (CHANGE_DIR_SCROLL_SPEED / 100))
                  + (tux.physic.get_velocity_x() * frame_ratio + tux.physic.get_acceleration_x() * frame_ratio * frame_ratio);
      // std::cerr << tux_pos_x << " " << final_scroll_x << " " << scroll_x << std::endl;

    }
    else
    {
      if (tux.physic.get_velocity_x() > 0 && scroll_x < tux_pos_x - (screen->w - X_SPACE))
        scroll_x = tux_pos_x - (screen->w - X_SPACE);
      else if (tux.physic.get_velocity_x() < 0 && scroll_x > tux_pos_x - X_SPACE && level->back_scrolling)
        scroll_x = tux_pos_x - X_SPACE;
      else
      {
        if (tux.dir == RIGHT && scroll_x < tux_pos_x - (screen->w - X_SPACE))
            scroll_x = tux_pos_x - (screen->w - X_SPACE);
        else if (tux.dir == LEFT && scroll_x > tux_pos_x - X_SPACE && level->back_scrolling)
            scroll_x = tux_pos_x - X_SPACE;
      }
    }
  }

  else /*no debug*/
  {
    if (tux.physic.get_velocity_x() > 0 && scroll_x < tux_pos_x - (screen->w - X_SPACE))
      scroll_x = tux_pos_x - (screen->w - X_SPACE);
    else if (tux.physic.get_velocity_x() < 0 && scroll_x > tux_pos_x - X_SPACE && level->back_scrolling)
      scroll_x = tux_pos_x - X_SPACE;
    else
    {
      if (tux.dir == RIGHT && scroll_x < tux_pos_x - (screen->w - X_SPACE))
          scroll_x = tux_pos_x - (screen->w - X_SPACE);
      else if (tux.dir == LEFT && scroll_x > tux_pos_x - X_SPACE && level->back_scrolling)
          scroll_x = tux_pos_x - X_SPACE;
    }

  }

  // this code prevent the screen to scroll before the start or after the level's end
  if(scroll_x < 0)
    scroll_x = 0;
#ifndef RES320X240
  if(scroll_x > level->width * 32 - screen->w)
    scroll_x = level->width * 32 - screen->w;
#else
  if(scroll_x > level->width * 32 - 640)
    scroll_x = level->width * 32 - 640;
#endif
}

void
World::collision_handler()
{
  // CO_BULLET & CO_BADGUY check
  for(unsigned int i = 0; i < bullets.size(); ++i)
    {
      for (BadGuys::iterator j = bad_guys.begin(); j != bad_guys.end(); ++j)
        {
          if((*j)->dying != DYING_NOT)
            continue;
          
          if(rectcollision(bullets[i].base, (*j)->base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              // collide with bad_guy first, since bullet_collision will
              // delete the bullet
              (*j)->collision(0, CO_BULLET);
              bullets[i].collision(CO_BADGUY);
              break; // bullet is invalid now, so break
            }
        }
    }

  /* CO_BADGUY & CO_BADGUY check */
  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    {
      if((*i)->dying != DYING_NOT)
        continue;
      
      BadGuys::iterator j = i;
      ++j;
      for (; j != bad_guys.end(); ++j)
        {
          if(j == i || (*j)->dying != DYING_NOT)
            continue;

          if(rectcollision((*i)->base, (*j)->base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              (*j)->collision(*i, CO_BADGUY);
              (*i)->collision(*j, CO_BADGUY);
            }
        }
    }

  if(tux.dying != DYING_NOT) return;
    
  // CO_BADGUY & CO_PLAYER check 
  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    {
      if((*i)->dying != DYING_NOT)
        continue;
      
      if(rectcollision_offset((*i)->base, tux.base, 0, 0))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          if (tux.previous_base.y < tux.base.y &&
              tux.previous_base.y + tux.previous_base.height 
              < (*i)->base.y + (*i)->base.height/2
              && !tux.invincible_timer.started())
            {
              (*i)->collision(&tux, CO_PLAYER, COLLISION_SQUISH);
            }
          else
            {
              tux.collision(*i, CO_BADGUY);
              (*i)->collision(&tux, CO_PLAYER, COLLISION_NORMAL);
            }
        }
    }

  // CO_UPGRADE & CO_PLAYER check
  for(unsigned int i = 0; i < upgrades.size(); ++i)
    {
      if(rectcollision(upgrades[i].base, tux.base))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          upgrades[i].collision(&tux, CO_PLAYER, COLLISION_NORMAL);
        }
    }
}

void
World::add_score(float x, float y, int s)
{
#ifdef RES320X240
  x=x/2;
#endif

  player_status.score += s;

  FloatingScore* new_floating_score = new FloatingScore();
  new_floating_score->init(x,y,s);
  floating_scores.push_back(new_floating_score);
}

void
World::add_bouncy_distro(float x, float y)
{
  BouncyDistro* new_bouncy_distro = new BouncyDistro();
  new_bouncy_distro->init(x,y);
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
  BrokenBrick* new_broken_brick = new BrokenBrick();
  new_broken_brick->init(tile, x, y, xm, ym);
  broken_bricks.push_back(new_broken_brick);
}

void
World::add_bouncy_brick(float x, float y)
{
  BouncyBrick* new_bouncy_brick = new BouncyBrick();
  new_bouncy_brick->init(x,y);
  bouncy_bricks.push_back(new_bouncy_brick);
}

BadGuy*
World::add_bad_guy(float x, float y, BadGuyKind kind, bool stay_on_platform)
{
  BadGuy* badguy = new BadGuy(x,y,kind, stay_on_platform);
  bad_guys.push_back(badguy);
  return badguy;
}

void
World::add_upgrade(float x, float y, Direction dir, UpgradeKind kind)
{
  Upgrade new_upgrade;
  new_upgrade.init(x,y,dir,kind);
  upgrades.push_back(new_upgrade);
}

void 
World::add_bullet(float x, float y, float xm, Direction dir)
{
  if(bullets.size() > MAX_BULLETS-1)
    return;

  Bullet new_bullet;
  new_bullet.init(x,y,xm,dir);
  bullets.push_back(new_bullet);
#ifndef NOSOUND
#ifndef GP2X  
  play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);
#else
  play_chunk(SND_SHOOT);
#endif
#endif
}

#ifndef NOSOUND
void
World::play_music(int musictype)
{
  currentmusic = musictype;
  switch(currentmusic) {
    case HURRYUP_MUSIC:
      music_manager->play_music(get_level()->get_level_music_fast());
      break;
    case LEVEL_MUSIC:
      music_manager->play_music(get_level()->get_level_music());
      break;
    case HERRING_MUSIC:
      music_manager->play_music(herring_song);
      break;
    default:
      music_manager->halt_music();
      break;
  }
}

int
World::get_music_type()
{
  return currentmusic;
}
#endif

/* Break a brick: */
void
World::trybreakbrick(float x, float y, bool small, Direction col_side)
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

          // TODO: don't handle this in a global way but per-tile...
          if (!counting_distros)
            {
              counting_distros = true;
              distro_counter = 5;
            }
          else
            {
              distro_counter--;
            }

          if (distro_counter <= 0)
            {
              counting_distros = false;
              plevel->change(x, y, TM_IA, tile->next_tile);
            }

#ifndef NOSOUND
#ifndef GP2X
          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
#else
	  play_chunk(SND_DISTRO);
#endif
#endif
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
#ifndef NOSOUND
#ifndef GP2X
          play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
#else
	  play_chunk(SND_BRICK);
#endif
#endif
          player_status.score = player_status.score + SCORE_BRICK;
        }
    }
  else if(tile->fullbox)
    tryemptybox(x, y, col_side);
}

/* Empty a box: */
void
World::tryemptybox(float x, float y, Direction col_side)
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
#ifndef NOSOUND
#ifndef GP2X
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
#else
      play_chunk(SND_DISTRO);
#endif
#endif
      player_status.score = player_status.score + SCORE_DISTRO;
      player_status.distros++;
      break;

    case 2: // Add an upgrade!
      if (tux.size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(posx, posy, col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add an iceflower: */
        add_upgrade(posx, posy, col_side, UPGRADE_ICEFLOWER);
#ifndef NOSOUND
#ifndef GP2X
      play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
#else
      play_chunk(SND_UPGRADE);
#endif
#endif
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
#ifndef NOSOUND
#ifndef GP2X
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
#else
      play_chunk(SND_DISTRO);
#endif
#endif

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
  // Bad guys: 
  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    {
      if ((*i)->base.x >= x - 32 && (*i)->base.x <= x + 32 &&
          (*i)->base.y >= y - 16 && (*i)->base.y <= y + 16)
        {
          (*i)->collision(&tux, CO_PLAYER, COLLISION_BUMP);
        }
    }

  // Upgrades:
  for (unsigned int i = 0; i < upgrades.size(); i++)
    {
      if (upgrades[i].base.height == 32 &&
          upgrades[i].base.x >= x - 32 && upgrades[i].base.x <= x + 32 &&
          upgrades[i].base.y >= y - 16 && upgrades[i].base.y <= y + 16)
        {
          upgrades[i].collision(&tux, CO_PLAYER, COLLISION_BUMP);
        }
    }
}

/* EOF */

