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
#include "viewport.h"
#include "display_manager.h"
#include "background.h"

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
  // add background
  activate_particle_systems();
  Background* bg = new Background(displaymanager);
  if(level->img_bkgd) {
    bg->set_image(level->img_bkgd, level->bkgd_speed);
  } else {
    bg->set_gradient(level->bkgd_top, level->bkgd_bottom);
  }
  gameobjects.push_back(bg);
  activate_objects();
  get_level()->load_song();

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
  activate_objects();
  activate_particle_systems();
  Background* bg = new Background(displaymanager);
  if(level->img_bkgd) {
    bg->set_image(level->img_bkgd, level->bkgd_speed);
  } else {
    bg->set_gradient(level->bkgd_top, level->bkgd_bottom);
  }
  gameobjects.push_back(bg);
  get_level()->load_song();

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
      tux.got_power = tux.FIRE_POWER;  // FIXME: add ice power to here
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

  for (Trampolines::iterator i = trampolines.begin(); i != trampolines.end(); ++i)
    delete *i;

  for (std::vector<_GameObject*>::iterator i = gameobjects.begin();
          i != gameobjects.end(); ++i) {
    Drawable* drawable = dynamic_cast<Drawable*> (*i);
    if(drawable)
      displaymanager.remove_drawable(drawable);
    delete *i;
  }

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
  currentmusic = LEVEL_MUSIC;
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
World::activate_objects()
{
  for (std::vector< ObjectData<TrampolineData> >::iterator i = level->trampoline_data.begin();
       i != level->trampoline_data.end();
       ++i)
  {
    add_object<Trampoline, ObjectData<TrampolineData> >(*i);
  }
}

void
World::activate_particle_systems()
{
  if (level->particle_system == "clouds")
    {
      gameobjects.push_back(new CloudParticleSystem(displaymanager));
    }
  else if (level->particle_system == "snow")
    {
      gameobjects.push_back(new SnowParticleSystem(displaymanager));
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
#if 0
  drawgradient(level->bkgd_top, level->bkgd_bottom);
  if(level->img_bkgd)
      level->draw_bg();
#endif
    
  /* Draw particle systems (background) */
  displaymanager.get_viewport().set_translation(Vector(scroll_x, scroll_y));
  displaymanager.draw();
  
  /* Draw background: */
  for (y = 0; y < VISIBLE_TILES_Y && y < level->height; ++y)
    {
      for (x = 0; x < VISIBLE_TILES_X; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32 - fmodf(scroll_y, 32),
                     level->bg_tiles[(int)y + (int)(scroll_y / 32)][(int)x + (int)(scroll_x / 32)]);
        }
    }

  /* Draw interactive tiles: */
  for (y = 0; y < VISIBLE_TILES_Y && y < level->height; ++y)
    {
      for (x = 0; x < VISIBLE_TILES_X; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32 - fmodf(scroll_y, 32),
                     level->ia_tiles[(int)y + (int)(scroll_y / 32)][(int)x + (int)(scroll_x / 32)]);
        }
    }

  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    (*i)->draw();

  for (Trampolines::iterator i = trampolines.begin(); i != trampolines.end(); ++i)
    (*i)->draw();

  tux.draw();

  for (unsigned int i = 0; i < bullets.size(); ++i)
    bullets[i].draw();

  for (unsigned int i = 0; i < upgrades.size(); ++i)
    upgrades[i].draw();

  /* Draw foreground: */
  for (y = 0; y < VISIBLE_TILES_Y && y < level->height; ++y)
    {
      for (x = 0; x < VISIBLE_TILES_X; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32 - fmodf(scroll_y, 32),
                     level->fg_tiles[(int)y + (int)(scroll_y / 32)][(int)x + (int)(scroll_x / 32)]);
        }
    }
}

void
World::action(double frame_ratio)
{
  tux.action(frame_ratio);
  tux.check_bounds(level->back_scrolling, (bool)level->hor_autoscroll_speed);
  scrolling(frame_ratio);

  for (unsigned int i = 0; i < bullets.size(); ++i)
    bullets[i].action(frame_ratio);
  
  for (unsigned int i = 0; i < upgrades.size(); i++)
    upgrades[i].action(frame_ratio);

  for (BadGuys::iterator i = bad_guys.begin(); i != bad_guys.end(); ++i)
    (*i)->action(frame_ratio);

  for (Trampolines::iterator i = trampolines.begin(); i != trampolines.end(); ++i)
     (*i)->action(frame_ratio);

  /* update particle systems */
  for(std::vector<_GameObject*>::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i)
    (*i)->action(frame_ratio);

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

  for(std::vector<_GameObject*>::iterator i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    if((*i)->is_valid() == false) {
      Drawable* drawable = dynamic_cast<Drawable*> (*i);
      if(drawable)
        displaymanager.remove_drawable(drawable);
      
      delete *i;
      i = gameobjects.erase(i);
    } else {
      ++i;
    }
  }
}

/* the space that it takes for the screen to start scrolling, regarding */
/* screen bounds (in pixels) */
// should be higher than screen->w/2 (400)
#define X_SPACE (500-16)
// should be less than screen->h/2 (300)
#define Y_SPACE 250

// the time it takes to move the camera (in ms)
#define CHANGE_DIR_SCROLL_SPEED 2000

/* This functions takes cares of the scrolling */
void World::scrolling(double frame_ratio)
{
  /* Y-axis scrolling */

  float tux_pos_y = tux.base.y + (tux.base.height/2);

  if(level->height > VISIBLE_TILES_Y-1 && !tux.dying)
    {
    if (scroll_y < tux_pos_y - (screen->h - Y_SPACE))
      scroll_y = tux_pos_y - (screen->h - Y_SPACE);
    else if (scroll_y > tux_pos_y - Y_SPACE)
      scroll_y = tux_pos_y - Y_SPACE;
    }

  // this code prevent the screen to scroll before the start or after the level's end
  if(scroll_y > level->height * 32 - screen->h)
    scroll_y = level->height * 32 - screen->h;
  if(scroll_y < 0)
    scroll_y = 0;

  /* X-axis scrolling */

  /* Auto scrolling */
  if(level->hor_autoscroll_speed)
  {
    scroll_x += level->hor_autoscroll_speed * frame_ratio;
    return;
  }


  /* Horizontal backscrolling */
  float tux_pos_x = tux.base.x + (tux.base.width/2);

  if(tux.old_dir != tux.dir && level->back_scrolling)
    scrolling_timer.start(CHANGE_DIR_SCROLL_SPEED);

  bool right = false;
  bool left = false;
  if (tux.physic.get_velocity_x() > 0)
    right = true;
  else if (tux.physic.get_velocity_x() < 0)
    left = true;
  else
    {
    if (tux.dir == RIGHT)
      right = true;
    else
      left = true;
    }

  if(scrolling_timer.check())
  {
    float final_scroll_x;
    float constant1;
    float constant2;
    if (right)
      final_scroll_x = tux_pos_x - (screen->w - X_SPACE);
    else
      final_scroll_x = tux_pos_x - X_SPACE;

    if((tux.physic.get_velocity_x() > 0 && tux.dir == RIGHT) || (tux.physic.get_velocity_x() < 0 && tux.dir == LEFT))
    {
      constant1 = 1.0;
      constant2 = .4;
    }
    else
    {
      constant1 = 0.;
      constant2 = 0.;
    }
    
    float number = 2.5/(frame_ratio * CHANGE_DIR_SCROLL_SPEED/1000)*exp((CHANGE_DIR_SCROLL_SPEED-scrolling_timer.get_left())/1400.);
    if(left) number *= -1.;

    scroll_x += number
	    + constant1 * tux.physic.get_velocity_x() * frame_ratio
	    + constant2 * tux.physic.get_acceleration_x() * frame_ratio * frame_ratio;

    if ((right && final_scroll_x - scroll_x < 0) || (left && final_scroll_x - scroll_x > 0))
      scroll_x = final_scroll_x;
    
  }
  else
  {
    if (right && scroll_x < tux_pos_x - (screen->w - X_SPACE))
      scroll_x = tux_pos_x - (screen->w - X_SPACE);
    else if (left && scroll_x > tux_pos_x - X_SPACE && level->back_scrolling)
      scroll_x = tux_pos_x - X_SPACE;
  }

  // this code prevent the screen to scroll before the start or after the level's end
  if(scroll_x > level->width * 32 - screen->w)
    scroll_x = level->width * 32 - screen->w;
  if(scroll_x < 0)
    scroll_x = 0;
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
              (*j)->collision(&bullets[i], CO_BULLET);
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

  // CO_TRAMPOLINE & (CO_PLAYER or CO_BADGUY)
  for (Trampolines::iterator i = trampolines.begin(); i != trampolines.end(); ++i)
  {
    if (rectcollision((*i)->base, tux.base))
    {
      if (tux.previous_base.y < tux.base.y &&
          tux.previous_base.y + tux.previous_base.height 
          < (*i)->base.y + (*i)->base.height/2)
      {
        (*i)->collision(&tux, CO_PLAYER, COLLISION_SQUISH);
      }
      else if (tux.previous_base.y <= tux.base.y)
      {
        tux.collision(*i, CO_TRAMPOLINE);
        (*i)->collision(&tux, CO_PLAYER, COLLISION_NORMAL);
      }
    }
  }
}

void
World::add_score(const Vector& pos, int s)
{
  player_status.score += s;

  gameobjects.push_back(new FloatingScore(displaymanager, pos, s));
}

void
World::add_bouncy_distro(const Vector& pos)
{
  gameobjects.push_back(new BouncyDistro(displaymanager, pos));
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
  gameobjects.push_back(new BrokenBrick(displaymanager, tile, pos, movement));
}

void
World::add_bouncy_brick(const Vector& pos)
{
  gameobjects.push_back(new BouncyBrick(displaymanager, pos));
}

BadGuy*
World::add_bad_guy(float x, float y, BadGuyKind kind, bool stay_on_platform)
{
  BadGuy* badguy = new BadGuy(x,y,kind, stay_on_platform);
  bad_guys.push_back(badguy);
  return badguy;
}

template<class T, class U>
T*
World::add_object(U data)
{
  T* tobject = new T(data);

  if (data.type == OBJ_TRAMPOLINE)
    trampolines.push_back(tobject);

  return tobject;
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
  if(tux.got_power == tux.FIRE_POWER)
    {
    if(bullets.size() > MAX_FIRE_BULLETS-1)
      return;
    }
  else if(tux.got_power == tux.ICE_POWER)
    {
    if(bullets.size() > MAX_ICE_BULLETS-1)
      return;
    }

  Bullet new_bullet;
  if(tux.got_power == tux.FIRE_POWER)
    new_bullet.init(x,y,xm,dir, FIRE_BULLET);
  else if(tux.got_power == tux.ICE_POWER)
    new_bullet.init(x,y,xm,dir, ICE_BULLET);
  bullets.push_back(new_bullet);
  
  play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);
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
      if (tux.size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(posx, posy, col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add a fireflower: */
        add_upgrade(posx, posy, col_side, UPGRADE_FIREFLOWER);
      play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
      break;
    
    case 5: // Add an ice flower upgrade!
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

