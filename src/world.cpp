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
#include "gameobjs.h"
#include "camera.h"
#include "display_manager.h"
#include "background.h"
#include "tilemap.h"

Surface* img_distro[4];

World* World::current_ = 0;

World::World(const std::string& filename, int level_nr)
  : level(0), tux(0), background(0), camera(0)
{
  // FIXME: Move this to action and draw and everywhere else where the
  // world calls child functions
  current_ = this;

  tux = new Player(displaymanager);
  add_object(tux);
  
  level = new Level();
  camera = new Camera(tux, level);
  add_object(camera);                 

  if(level_nr >= 0) {
    level->load(filename, level_nr, this);
  } else {
    level->load(filename, this);
  }
  tux->move(level->start_pos);
  
  set_defaults();

  level->load_gfx();
  // add background
  activate_particle_systems();
  background = new Background(displaymanager);
  if(level->img_bkgd) {
    background->set_image(level->img_bkgd, level->bkgd_speed);
  } else {
    background->set_gradient(level->bkgd_top, level->bkgd_bottom);
  }
  add_object(background);

  // add tilemap
  add_object(new TileMap(displaymanager, level));
  level->load_song();

  apply_bonuses();
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
      tux->got_power = Player::FIRE_POWER;  // FIXME: add ice power to here
      // fall through
                                                                                
    case PlayerStatus::GROWUP_BONUS:
      tux->grow();
      break;
    }
}

World::~World()
{
  for (std::vector<GameObject*>::iterator i = gameobjects.begin();
          i != gameobjects.end(); ++i) {
    delete *i;
  }

  delete level;

  current_ = 0;
}

void
World::set_defaults()
{
  player_status.score_multiplier = 1;

  counting_distros = false;
  distro_counter = 0;

  /* set current song/music */
  currentmusic = LEVEL_MUSIC;
}

void
World::add_object(GameObject* object)
{
  // XXX hack for now until new collision code is ready
  BadGuy* badguy = dynamic_cast<BadGuy*> (object);
  if(badguy)
    bad_guys.push_back(badguy);
  Bullet* bullet = dynamic_cast<Bullet*> (object);
  if(bullet)
    bullets.push_back(bullet);
  Upgrade* upgrade = dynamic_cast<Upgrade*> (object);
  if(upgrade)
    upgrades.push_back(upgrade);
  Trampoline* trampoline = dynamic_cast<Trampoline*> (object);
  if(trampoline)
    trampolines.push_back(trampoline);
  FlyingPlatform* flying_platform = dynamic_cast<FlyingPlatform*> (object);
  if(flying_platform)
    flying_platforms.push_back(flying_platform);

  gameobjects.push_back(object);
}

void
World::parse_objects(lisp_object_t* cur)
{
  while(!lisp_nil_p(cur)) {
    lisp_object_t* data = lisp_car(cur);
    std::string object_type = lisp_symbol(lisp_car(data));
    
    LispReader reader(lisp_cdr(data));

    if(object_type == "trampoline") {
      add_object(new Trampoline(displaymanager, reader));
    }
    else if(object_type == "flying-platform") {
      add_object(new FlyingPlatform(displaymanager, reader));
    }
    else {
      BadGuyKind kind = badguykind_from_string(object_type);
      add_object(new BadGuy(displaymanager, kind, reader));
    }
      
    cur = lisp_cdr(cur);
  } 
}

void
World::activate_particle_systems()
{
  if (level->particle_system == "clouds")
    {
      add_object(new CloudParticleSystem(displaymanager));
    }
  else if (level->particle_system == "snow")
    {
      add_object(new SnowParticleSystem(displaymanager));
    }
  else if (level->particle_system != "")
    {
      st_abort("unknown particle system specified in level", "");
    }
}

void
World::draw()
{
  /* Draw objects */
  displaymanager.draw(*camera);
}

void
World::action(float elapsed_time)
{
  tux->check_bounds(*camera,
      level->back_scrolling, (bool)level->hor_autoscroll_speed);
    
  /* update objects (don't use iterators here, because the list might change
   * during the iteration)
   */
  for(size_t i = 0; i < gameobjects.size(); ++i)
    if(gameobjects[i]->is_valid())
      gameobjects[i]->action(elapsed_time);

  /* Handle all possible collisions. */
  collision_handler();
 
  /** cleanup marked objects */
  for(std::vector<GameObject*>::iterator i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    if((*i)->is_valid() == false) {
      Drawable* drawable = dynamic_cast<Drawable*> (*i);
      if(drawable)
        displaymanager.remove_drawable(drawable);
      BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
      if(badguy) {
        bad_guys.erase(std::remove(bad_guys.begin(), bad_guys.end(), badguy),
            bad_guys.end());
      }
      Bullet* bullet = dynamic_cast<Bullet*> (*i);
      if(bullet) {
        bullets.erase(
            std::remove(bullets.begin(), bullets.end(), bullet),
            bullets.end());
      }
      Upgrade* upgrade = dynamic_cast<Upgrade*> (*i);
      if(upgrade) {
        upgrades.erase(
            std::remove(upgrades.begin(), upgrades.end(), upgrade),
            upgrades.end());
      }
      Trampoline* trampoline = dynamic_cast<Trampoline*> (*i);
      if(trampoline) {
        trampolines.erase(
            std::remove(trampolines.begin(), trampolines.end(), trampoline),
            trampolines.end());
      }
      FlyingPlatform* flying_platform= dynamic_cast<FlyingPlatform*> (*i);
      if(flying_platform) {
        flying_platforms.erase(
            std::remove(flying_platforms.begin(), flying_platforms.end(), flying_platform),
            flying_platforms.end());
      }
      
      delete *i;
      i = gameobjects.erase(i);
    } else {
      ++i;
    }
  }
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
          
          if(rectcollision(bullets[i]->base, (*j)->base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              (*j)->collision(bullets[i], CO_BULLET, COLLISION_NORMAL);
              bullets[i]->collision(CO_BADGUY);
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

  if(tux->dying != DYING_NOT) return;
    
  // CO_BADGUY & CO_PLAYER check 
  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    {
      if((*i)->dying != DYING_NOT)
        continue;
      
      if(rectcollision_offset((*i)->base, tux->base, 0, 0))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          if (tux->previous_base.y < tux->base.y &&
              tux->previous_base.y + tux->previous_base.height 
              < (*i)->base.y + (*i)->base.height/2
              && !tux->invincible_timer.started())
            {
              (*i)->collision(tux, CO_PLAYER, COLLISION_SQUISH);
            }
          else
            {
              tux->collision(*i, CO_BADGUY);
              (*i)->collision(tux, CO_PLAYER, COLLISION_NORMAL);
            }
        }
    }

  // CO_UPGRADE & CO_PLAYER check
  for(unsigned int i = 0; i < upgrades.size(); ++i)
    {
      if(rectcollision(upgrades[i]->base, tux->base))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          upgrades[i]->collision(tux, CO_PLAYER, COLLISION_NORMAL);
        }
    }

  // CO_TRAMPOLINE & (CO_PLAYER or CO_BADGUY)
  for (Trampolines::iterator i = trampolines.begin(); i != trampolines.end(); ++i)
  {
    if (rectcollision((*i)->base, tux->base))
    {
      if (tux->previous_base.y < tux->base.y &&
          tux->previous_base.y + tux->previous_base.height 
          < (*i)->base.y + (*i)->base.height/2)
      {
        (*i)->collision(tux, CO_PLAYER, COLLISION_SQUISH);
      }
      else if (tux->previous_base.y <= tux->base.y)
      {
        tux->collision(*i, CO_TRAMPOLINE);
        (*i)->collision(tux, CO_PLAYER, COLLISION_NORMAL);
      }
    }
  }

  // CO_FLYING_PLATFORM & (CO_PLAYER or CO_BADGUY)
  for (FlyingPlatforms::iterator i = flying_platforms.begin(); i != flying_platforms.end(); ++i)
  {
    if (rectcollision((*i)->base, tux->base))
    {
      if (tux->previous_base.y < tux->base.y &&
          tux->previous_base.y + tux->previous_base.height 
          < (*i)->base.y + (*i)->base.height/2)
      {
        (*i)->collision(tux, CO_PLAYER, COLLISION_SQUISH);
        tux->collision(*i, CO_FLYING_PLATFORM);
      }
/*      else if (tux->previous_base.y <= tux->base.y)
      {
      }*/
    }
  }
}

void
World::add_score(const Vector& pos, int s)
{
  player_status.score += s;

  add_object(new FloatingScore(displaymanager, pos, s));
}

void
World::add_bouncy_distro(const Vector& pos)
{
  add_object(new BouncyDistro(displaymanager, pos));
}

void
World::add_broken_brick(const Vector& pos, Tile* tile)
{
  add_broken_brick_piece(pos, Vector(-1, -4), tile);
  add_broken_brick_piece(pos + Vector(0, 16), Vector(-1.5, -3), tile);

  add_broken_brick_piece(pos + Vector(16, 0), Vector(1, -4), tile);
  add_broken_brick_piece(pos + Vector(16, 16), Vector(1.5, -3), tile);
}

void
World::add_broken_brick_piece(const Vector& pos, const Vector& movement,
    Tile* tile)
{
  add_object(new BrokenBrick(displaymanager, tile, pos, movement));
}

void
World::add_bouncy_brick(const Vector& pos)
{
  add_object(new BouncyBrick(displaymanager, pos));
}

BadGuy*
World::add_bad_guy(float x, float y, BadGuyKind kind)
{
  BadGuy* badguy = new BadGuy(displaymanager, kind, x, y);
  add_object(badguy);
  return badguy;
}

void
World::add_upgrade(const Vector& pos, Direction dir, UpgradeKind kind)
{
  add_object(new Upgrade(displaymanager, pos, dir, kind));
}

bool
World::add_bullet(const Vector& pos, float xm, Direction dir)
{
  if(tux->got_power == Player::FIRE_POWER)
    {
    if(bullets.size() > MAX_FIRE_BULLETS-1)
      return false;
    }
  else if(tux->got_power == Player::ICE_POWER)
    {
    if(bullets.size() > MAX_ICE_BULLETS-1)
      return false;
    }

  Bullet* new_bullet = 0;
  if(tux->got_power == Player::FIRE_POWER)
    new_bullet = new Bullet(displaymanager, pos, xm, dir, FIRE_BULLET);
  else if(tux->got_power == Player::ICE_POWER)
    new_bullet = new Bullet(displaymanager, pos, xm, dir, ICE_BULLET);
  else
    st_abort("wrong bullet type.", "");
  add_object(new_bullet);
  
  play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);

  return true;
}

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

/* Break a brick: */
bool
World::trybreakbrick(float x, float y, bool small)
{
  Level* plevel = get_level();
  
  Tile* tile = gettile(x, y);
  if (tile->brick)
    {
      if (tile->data > 0)
        {
          /* Get a distro from it: */
          add_bouncy_distro(
              Vector(((int)(x + 1) / 32) * 32, (int)(y / 32) * 32));

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

          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
          player_status.score = player_status.score + SCORE_DISTRO;
          player_status.distros++;
          return true;
        }
      else if (!small)
        {
          /* Get rid of it: */
          plevel->change(x, y, TM_IA, tile->next_tile);
          
          /* Replace it with broken bits: */
          add_broken_brick(Vector(
                                 ((int)(x + 1) / 32) * 32,
                                 (int)(y / 32) * 32), tile);
          
          /* Get some score: */
          play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
          player_status.score = player_status.score + SCORE_BRICK;
          
          return true;
        }
    }

  return false;
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
      add_bouncy_distro(Vector(posx, posy));
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
      player_status.score = player_status.score + SCORE_DISTRO;
      player_status.distros++;
      break;

    case 2: // Add a fire flower upgrade!
      if (tux->size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add a fireflower: */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_FIREFLOWER);
      play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
      break;
    
    case 5: // Add an ice flower upgrade!
      if (tux->size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add an iceflower: */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_ICEFLOWER);
      play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
      break;

    case 3: // Add a golden herring
      add_upgrade(Vector(posx, posy), col_side, UPGRADE_HERRING);
      break;

    case 4: // Add a 1up extra
      add_upgrade(Vector(posx, posy), col_side, UPGRADE_1UP);
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
          add_bouncy_distro(Vector(((int)(x + 1) / 32) * 32,
                                  (int)(y / 32) * 32));
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
          (*i)->collision(tux, CO_PLAYER, COLLISION_BUMP);
        }
    }

  // Upgrades:
  for (unsigned int i = 0; i < upgrades.size(); i++)
    {
      if (upgrades[i]->base.height == 32 &&
          upgrades[i]->base.x >= x - 32 && upgrades[i]->base.x <= x + 32 &&
          upgrades[i]->base.y >= y - 16 && upgrades[i]->base.y <= y + 16)
        {
          upgrades[i]->collision(tux, CO_PLAYER, COLLISION_BUMP);
        }
    }
}

/* EOF */

