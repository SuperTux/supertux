//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
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
#include <cmath>

#include "app/globals.h"
#include "defines.h"
#include "special/sprite_manager.h"
#include "utils/lispwriter.h"
#include "badguy.h"
#include "tile.h"
#include "resources.h"
#include "camera.h"
#include "level.h"
#include "sector.h"
#include "tilemap.h"
#include "statistics.h"
#include "badguy_specs.h"

#define BADGUY_WALK_SPEED .8f
#define WINGLING_FLY_SPEED 1.6f

BadGuyKind  badguykind_from_string(const std::string& str)
{
  if (str == "jumpy" || str == "money") // was "money" in ancient versions
    return BAD_JUMPY;
  else if (str == "mriceblock" || str == "laptop") // was "laptop" in ancient versions
    return BAD_MRICEBLOCK;
  else if (str == "mrbomb")
    return BAD_MRBOMB;
  else if (str == "stalactite")
    return BAD_STALACTITE;
  else if (str == "flame")
    return BAD_FLAME;
  else if (str == "fish")
    return BAD_FISH;
  else if (str == "flamefish")
    return BAD_FLAMEFISH;
  else if (str == "bouncingsnowball")
    return BAD_BOUNCINGSNOWBALL;
  else if (str == "flyingsnowball")
    return BAD_FLYINGSNOWBALL;
  else if (str == "spiky")
    return BAD_SPIKY;
  else if (str == "snowball" || str == "bsod") // was "bsod" in ancient versions
    return BAD_SNOWBALL;
  else if (str == "wingling")
    return BAD_WINGLING;
  else if (str == "walkingtree")
    return BAD_WALKINGTREE;
  else if(str == "bomb")  // not to be used as a real bad guys
      return BAD_BOMB;
  else
    {
      return BAD_INVALID;
    }
}

std::string badguykind_to_string(BadGuyKind kind)
{
  switch(kind)
    {
    case BAD_JUMPY:
      return "jumpy";
      break;
    case BAD_MRICEBLOCK:
      return "mriceblock";
      break;
    case BAD_MRBOMB:
      return "mrbomb";
      break;
    case BAD_STALACTITE:
      return "stalactite";
      break;
    case BAD_FLAME:
      return "flame";
      break;
    case BAD_FISH:
      return "fish";
      break;
    case BAD_FLAMEFISH:
      return "flamefish";
      break;
    case BAD_BOUNCINGSNOWBALL:
      return "bouncingsnowball";
      break;
    case BAD_FLYINGSNOWBALL:
      return "flyingsnowball";
      break;
    case BAD_SPIKY:
      return "spiky";
      break;
    case BAD_SNOWBALL:
      return "snowball";
      break;
    case BAD_WINGLING:
      return "wingling";
      break;
    case BAD_WALKINGTREE:
      return "walkingtree";
    case BAD_BOMB:  // not to be used as a real bad guys
      return "bomb";
      break;
    default:
      return "snowball";
    }
}

BadGuy::BadGuy(BadGuyKind kind_, LispReader& lispreader)
  : removable(false), squishcount(0)
{
  lispreader.read_float("x", start_position.x);
  lispreader.read_float("y", start_position.y);

  kind     = kind_;

  stay_on_platform = false;
  lispreader.read_bool("stay-on-platform", stay_on_platform);

  init();
}

BadGuy::BadGuy(BadGuyKind kind_, float x, float y)
  : removable(false), squishcount(0)
{
  start_position.x = x;
  start_position.y = y;
  stay_on_platform = false;

  kind     = kind_;
  
  init();
}

BadGuy::~BadGuy()
{
}

void
BadGuy::init()
{
  base.x = start_position.x;
  base.y = start_position.y;
  base.width  = 32;
  base.height = 32;
  
  mode     = NORMAL;
  old_base = base;
  dir      = LEFT;
  seen     = false;
  animation_offset = 0;
  target.x = target.y = -1;
  physic.reset();
  frozen_timer.init(true);
  timer.init(true);

  specs = badguyspecs_manager->load(badguykind_to_string(kind));

  // if we're in a solid tile at start correct that now
  if(Sector::current()) {
  if(kind != BAD_FLAME && kind != BAD_FISH && kind != BAD_FLAMEFISH && collision_object_map(base)) 
    {
      std::cout << "Warning: badguy started in wall: kind: " << badguykind_to_string(kind) 
                << " pos: (" << base.x << ", " << base.y << ")" << std::endl;
      while(collision_object_map(base))
        --base.y;
    }
  }
}

void
BadGuy::write(LispWriter& writer)
{
  writer.start_list(badguykind_to_string(kind));

  writer.write_float("x", base.x);
  writer.write_float("y", base.y);
  writer.write_bool("stay-on-platform", stay_on_platform);  

  writer.end_list(badguykind_to_string(kind));
}

void
BadGuy::activate(Direction activation_dir)
{
  mode     = NORMAL;
  animation_offset = 0;
  target.x = target.y = -1;
  physic.reset();
  frozen_timer.init(true);
  timer.init(true);

  seen = true;

  dir = activation_dir;
  float dirsign = activation_dir == LEFT ? -1 : 1;

  set_action("left", "right");
  if(kind == BAD_MRBOMB) {
    physic.set_velocity(dirsign * BADGUY_WALK_SPEED, 0);
  } else if (kind == BAD_MRICEBLOCK) {
    physic.set_velocity(dirsign * BADGUY_WALK_SPEED, 0);
  } else if(kind == BAD_JUMPY) {
    set_action("left-up", "right-up");
  } else if(kind == BAD_BOMB) {
    set_action("ticking-left", "ticking-right");
    // hack so that the bomb doesn't hurt until it expldes...           
    dying = DYING_SQUISHED;
  } else if(kind == BAD_FLAME) {
    angle = 0;
    physic.enable_gravity(false);
    set_action("normal", "normal");
  } else if(kind == BAD_BOUNCINGSNOWBALL) {
    physic.set_velocity(dirsign * 1.3, 0);
  } else if(kind == BAD_STALACTITE) {
    physic.enable_gravity(false);
    set_action("normal", "normal");
  } else if(kind == BAD_FISH) {
    set_action("normal", "normal");
    physic.enable_gravity(true);
  } else if(kind == BAD_FLAMEFISH) {
    set_action("normal", "normal");
    physic.enable_gravity(true);
  } else if(kind == BAD_FLYINGSNOWBALL) {
    physic.enable_gravity(false);
  } else if(kind == BAD_SPIKY) {
    physic.set_velocity(dirsign * BADGUY_WALK_SPEED, 0);
  } else if(kind == BAD_SNOWBALL) {
    physic.set_velocity(dirsign * BADGUY_WALK_SPEED, 0);
  } else if(kind == BAD_WINGLING) {
    physic.set_velocity(dirsign * WINGLING_FLY_SPEED, 0);
    physic.enable_gravity(false);
    set_action("left", "left");
  } else if (kind == BAD_WALKINGTREE) {
    // TODO: why isn't the height/width being set properly in set_action?
    physic.set_velocity(dirsign * BADGUY_WALK_SPEED, 0);
    mode = BGM_BIG;
    set_action("left", "left");
    base.width = 66;
    base.height = 66;
  }
}

Surface*
BadGuy::get_image()
{
// Set action as the "default" one.
specs->sprite->set_action("left");
if(BAD_JUMPY)
  specs->sprite->set_action("left-up");
else if(kind == BAD_BOMB)
  specs->sprite->set_action("ticking-left");
else if(kind == BAD_FLAME)
  specs->sprite->set_action("normal");
else if(kind == BAD_STALACTITE)
  specs->sprite->set_action("normal");
else if(kind == BAD_FISH)
  specs->sprite->set_action("normal");
else if(kind == BAD_FLAMEFISH)
  specs->sprite->set_action("normal");

return specs->sprite->get_frame(0);
}

void
BadGuy::action_mriceblock(double elapsed_time)
{
  Player& tux = *Sector::current()->player;

  if(mode != HELD)
    fall();
  
  /* Move left/right: */
  if (mode != HELD)
    {
      // move
      physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
      if (dying != DYING_FALLING)
        collision_swept_object_map(&old_base,&base);
    }
  else if (mode == HELD)
    { /* FIXME: The pbad object shouldn't know about pplayer objects. */
      /* If we're holding the iceblock */
      dir = tux.dir;
      if(tux.size == SMALL)
        {
        if(dir == RIGHT)
          base.x = tux.base.x + 24;
        else // dir == LEFT
          base.x = tux.base.x - 12;
        base.y = tux.base.y + tux.base.height/1.5 - base.height;
        }
      else // TUX == BIG
        {
        if(dir == RIGHT)
          base.x = tux.base.x + 24;
        else // dir == LEFT
          base.x = tux.base.x - 4;
        base.y = tux.base.y + tux.base.height/1.5 - base.height;
        }

      if(collision_object_map(base))
        {
          base.x = tux.base.x;
          base.y = tux.base.y + tux.base.height/1.5 - base.height;
        }

      if(tux.input.fire != DOWN) /* SHOOT! */
        {
          if(dir == LEFT)
            base.x = tux.base.x - base.width;
          else
            base.x = tux.base.x + tux.base.width;
          old_base = base;

          mode=KICK;
          tux.kick_timer.start(KICKING_TIME);
          set_action("flat-left", "flat-right");
          physic.set_velocity_x((dir == LEFT) ? -3.5 : 3.5);
          SoundManager::get()->play_sound(IDToSound(SND_KICK), this, Sector::current()->player->get_pos());
        }
    }

  if (!dying)
    {
      int changed = dir;
      check_horizontal_bump();
      if(mode == KICK && changed != dir)
        {
          SoundManager::get()->play_sound(IDToSound(SND_RICOCHET), get_pos(), Sector::current()->player->get_pos());
        }
    }

  /* Handle mode timer: */
  if (mode == FLAT)
    {
      if(!timer.check())
        {
          mode = NORMAL;
          set_action("left", "right");
          physic.set_velocity( (dir == LEFT) ? -.8 : .8, 0);
        }
    }
}

void
BadGuy::check_horizontal_bump(bool checkcliff)
{
    float halfheight = base.height / 2;
    if (dir == LEFT && issolid( base.x, base.y + halfheight))
    {
        if (kind == BAD_MRICEBLOCK && mode == KICK)
            {
            Sector::current()->trybreakbrick(Vector(base.x, base.y + halfheight), false);
            Sector::current()->tryemptybox(Vector(base.x, base.y + halfheight), dir);
            }
            
        dir = RIGHT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }
    if (dir == RIGHT && issolid( base.x + base.width, base.y + halfheight))
    {
        if (kind == BAD_MRICEBLOCK && mode == KICK)
            {
            Sector::current()->trybreakbrick(
                Vector(base.x + base.width, base.y + halfheight), false);
            Sector::current()->tryemptybox(
                Vector(base.x + base.width, base.y + halfheight), dir);
            }
            
        dir = LEFT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }

    // don't check for cliffs when we're falling
    if(!checkcliff)
        return;
    if(!issolid(base.x + base.width/2, base.y + base.height))
        return;
    
    if(dir == LEFT && !issolid(base.x, (int) base.y + base.height + halfheight))
    {
        dir = RIGHT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }
    if(dir == RIGHT && !issolid(base.x + base.width,
                (int) base.y + base.height + halfheight))
    {
        dir = LEFT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }
}

void
BadGuy::fall()
{
  /* Fall if we get off the ground: */
  if (dying != DYING_FALLING)
    {
      if (!issolid(base.x+base.width/2, base.y + base.height))
        {
          // not solid below us? enable gravity
          physic.enable_gravity(true);
        }
      else
        {
          /* Land: */
          if (physic.get_velocity_y() < 0)
            {
              base.y = int((base.y + base.height)/32) * 32 - base.height;
              physic.set_velocity_y(0);
            }
          // no gravity anymore please
          physic.enable_gravity(false);

          if (stay_on_platform && mode == NORMAL)
            {
              if (!issolid(base.x + ((dir == LEFT) ? 0 : base.width),
                           base.y + base.height))
                {
                  if (dir == LEFT)
                  {
                    dir = RIGHT;
                    physic.set_velocity_x(fabsf(physic.get_velocity_x()));
                  } 
                  else
                  {
                    dir = LEFT;
                    physic.set_velocity_x(-fabsf(physic.get_velocity_x()));
                  }
                }
            }
        }
    }
  else
    {
      physic.enable_gravity(true);
    }
}

void
BadGuy::action_jumpy(double elapsed_time)
{
  if(frozen_timer.check())
    {
    set_action("left-iced", "right-iced");
    return;
    }

  const float vy = physic.get_velocity_y();

  // XXX: These tests *should* use location from ground, not velocity
  if (fabsf(vy) > 5.6f)
    set_action("left-down", "right-down");
  else if (fabsf(vy) > 5.3f)
    set_action("left-middle", "right-middle");
  else
    set_action("left-up", "right-up");

  Player& tux = *Sector::current()->player;

  static const float JUMPV = 6;
    
  fall();
  // jump when on ground
  if(dying == DYING_NOT && issolid(base.x, base.y+32))
    {
      physic.set_velocity_y(JUMPV);
      physic.enable_gravity(true);

      mode = JUMPY_JUMP;
    }
  else if(mode == JUMPY_JUMP)
    {
      mode = NORMAL;
    }

  // set direction based on tux
  if(dying == DYING_NOT)
    {
    if(tux.base.x > base.x)
      dir = RIGHT;
    else
      dir = LEFT;
    }

  // move
  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if(dying == DYING_NOT)
    collision_swept_object_map(&old_base, &base);
}

void
BadGuy::action_mrbomb(double elapsed_time)
{
  if(frozen_timer.check())
    {
    set_action("iced-left", "iced-right");
    return;
    }

  if (dying == DYING_NOT)
    check_horizontal_bump(true);

  fall();

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base); 
}

void
BadGuy::action_bomb(double elapsed_time)
{
  static const int TICKINGTIME = 1000;
  static const int EXPLODETIME = 1000;
    
  fall();

  if(mode == NORMAL) {
    mode = BOMB_TICKING;
    timer.start(TICKINGTIME);
  } else if(!timer.check()) {
    if(mode == BOMB_TICKING) {
      mode = BOMB_EXPLODE;
      set_action("explosion", "explosion");
      dying = DYING_NOT; // now the bomb hurts
      timer.start(EXPLODETIME);

      SoundManager::get()->play_sound(IDToSound(SND_EXPLODE), this, Sector::current()->player->get_pos());
    } else if(mode == BOMB_EXPLODE) {
      remove_me();
      return;
    }
  }

  // move
  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);                 
  collision_swept_object_map(&old_base,&base);
}

void
BadGuy::action_stalactite(double elapsed_time)
{
  Player& tux = *Sector::current()->player;

  static const int SHAKETIME = 800;
  static const int RANGE = 40;
    
  if(mode == NORMAL) {
    // start shaking when tux is below the stalactite and at least 40 pixels
    // near
    if(tux.base.x + 32 > base.x - RANGE && tux.base.x < base.x + 32 + RANGE
            && tux.base.y + tux.base.height > base.y
            && tux.dying == DYING_NOT) {
      timer.start(SHAKETIME);
      mode = STALACTITE_SHAKING;
    }
  } if(mode == STALACTITE_SHAKING) {
    base.x = old_base.x + (rand() % 6) - 3; // TODO this could be done nicer...
    if(!timer.check()) {
      mode = STALACTITE_FALL;
    }
  } else if(mode == STALACTITE_FALL) {
    fall();
    /* Destroy if we collides with land */
    if(issolid(base.x+base.width/2, base.y+base.height))
    {
      timer.start(2000);
      dying = DYING_SQUISHED;
      mode = FLAT;
      set_action("broken", "broken");
    }
  } else if(mode == FLAT) {
    fall();
  }

  // move
  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);

  if(dying == DYING_SQUISHED && !timer.check())
    remove_me();
}

void
BadGuy::action_flame(double elapsed_time)
{
    static const float radius = 100;
    static const float speed = 0.02;
    base.x = old_base.x + cos(angle) * radius;
    base.y = old_base.y + sin(angle) * radius;

    angle = fmodf(angle + elapsed_time * speed, 2*M_PI);
}

void
BadGuy::action_fish(double elapsed_time)
{
  if(frozen_timer.check())
    {
    if(physic.get_velocity_y() < 0)
      set_action("iced-down", "iced-down");
    else
      set_action("iced", "iced");

    return;
    }

  static const float JUMPV = 6;
  static const int WAITTIME = 1000;
    
  // go in wait mode when back in water
  if(dying == DYING_NOT 
      && gettile(base.x, base.y + base.height)
      && gettile(base.x, base.y + base.height)->attributes & Tile::WATER
      && physic.get_velocity_y() <= 0 && mode == NORMAL)
    {
      mode = FISH_WAIT;
      set_action("hide", "hide");
      physic.set_velocity(0, 0);
      physic.enable_gravity(false);
      timer.start(WAITTIME);
    }
  else if(mode == FISH_WAIT && !timer.check())
    {
      // jump again
      set_action("normal", "normal");
      mode = NORMAL;
      physic.set_velocity(0, JUMPV);
      physic.enable_gravity(true);
    }

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if(dying == DYING_NOT)
    collision_swept_object_map(&old_base, &base);

  if(physic.get_velocity_y() < 0)
    {
      set_action("down", "down");
    }
}

void
BadGuy::action_bouncingsnowball(double elapsed_time)
{
  static const float JUMPV = 4.5;
    
  fall();

  // jump when on ground
  if(dying == DYING_NOT && issolid(base.x, base.y+32))
    {
      physic.set_velocity_y(JUMPV);
      physic.enable_gravity(true);
    }                                                     
  else
    {
      mode = NORMAL;
    }

  // check for right/left collisions
  check_horizontal_bump();

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if(dying == DYING_NOT)
    collision_swept_object_map(&old_base, &base);

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    remove_me();
}

void
BadGuy::action_flyingsnowball(double elapsed_time)
{
  static const float FLYINGSPEED = 1;
  static const int DIRCHANGETIME = 1000;
    
  // go into flyup mode if none specified yet
  if(dying == DYING_NOT && mode == NORMAL) {
    mode = FLY_UP;
    physic.set_velocity_y(FLYINGSPEED);
    timer.start(DIRCHANGETIME/2);
  }

  if(dying == DYING_NOT && !timer.check()) {
    if(mode == FLY_UP) {
      mode = FLY_DOWN;
      physic.set_velocity_y(-FLYINGSPEED);
    } else if(mode == FLY_DOWN) {
      mode = FLY_UP;
      physic.set_velocity_y(FLYINGSPEED);
    }
    timer.start(DIRCHANGETIME);
  }

  if(dying != DYING_NOT)
    physic.enable_gravity(true);

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if(dying == DYING_NOT || dying == DYING_SQUISHED)
    collision_swept_object_map(&old_base, &base);

  if(dying == DYING_NOT)
    {
    // set direction based on tux
    if(Sector::current()->player->base.x > base.x)
      dir = RIGHT;
    else
      dir = LEFT;
    }

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    remove_me();
}

void
BadGuy::action_spiky(double elapsed_time)
{
  if(frozen_timer.check())
    {
    set_action("iced-left", "iced-right");
    return;
    }

  if (dying == DYING_NOT)
    check_horizontal_bump();

  fall();

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);   
}

void
BadGuy::action_snowball(double elapsed_time)
{
  if (dying == DYING_NOT)
    check_horizontal_bump();

  fall();

  // jump when we're about to fall
  if (physic.get_velocity_y() == 0 && 
          !issolid(base.x+base.width/2, base.y + base.height)) {
    physic.enable_gravity(true);
    physic.set_velocity_y(2);
  }

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    remove_me();                                  
}

void
BadGuy::action_wingling(double elapsed_time)
{
  if (dying != DYING_NOT)
    physic.enable_gravity(true);
  else
  {
    Player& tux = *Sector::current()->player;
    int dirsign = physic.get_velocity_x() < 0 ? -1 : 1;

    if (fabsf(tux.base.x - base.x) < 150 && base.y < tux.base.y && tux.dying == DYING_NOT)
    {
      if (target.x < 0 && target.y < 0)
      {
        target.x = tux.base.x;
        target.y = tux.base.y;
        physic.set_velocity(dirsign * 1.5f, -2.25f);
      }
    }
    else if (base.y >= target.y - 16)
      physic.set_velocity(dirsign * WINGLING_FLY_SPEED, 0);
  }

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);


  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    remove_me();

  // TODO: Winglings should be removed after flying off the screen
}

void
BadGuy::action_walkingtree(double elapsed_time)
{
  if (dying == DYING_NOT)
    check_horizontal_bump();

  fall();

  physic.apply(elapsed_time, base.x, base.y, Sector::current()->gravity);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    remove_me();
}

void
BadGuy::action(float elapsed_time)
{
  float scroll_x = Sector::current()->camera->get_translation().x;
  float scroll_y = Sector::current()->camera->get_translation().y;
  
  // BadGuy fall below the ground
  if (base.y > Sector::current()->solids->get_height() * 32) {
    remove_me();
    return;
  }

  // Kill us if we landed on spikes
  if (dying == DYING_NOT
      && (kind != BAD_STALACTITE && kind != BAD_FLAME && kind != BAD_BOMB)
      && (isspike(base.x, base.y) || isspike(base.x + base.width, base.y)
      ||  isspike(base.x, base.y + base.height)
      ||  isspike(base.x + base.width, base.y + base.height)))
      {
         physic.set_velocity_y(3);
         kill_me(0);
      }

  if(!seen)
    {
    /* Activate badguys if they're just around the screen to avoid
     * the effect of having badguys suddenly popping up from nowhere.
     */
    if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
        start_position.x < scroll_x - base.width &&
        start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
        start_position.y < scroll_y + screen->h + Y_OFFSCREEN_DISTANCE)
      activate(RIGHT);
    else if (start_position.x > scroll_x + screen->w &&
        start_position.x < scroll_x + screen->w + X_OFFSCREEN_DISTANCE &&
        start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
        start_position.y < scroll_y + screen->h + Y_OFFSCREEN_DISTANCE)
      activate(LEFT);
    else if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
        start_position.x < scroll_x + screen->w + X_OFFSCREEN_DISTANCE &&
        ((start_position.y > scroll_y + screen->h &&
        start_position.y < scroll_y + screen->h + Y_OFFSCREEN_DISTANCE) ||
        (start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
        start_position.y < scroll_y)))
        {
        if(start_position.x < scroll_x - screen->w/2)
          activate(RIGHT);
        else
          activate(LEFT);
        }
    /* Special case for badguys on start of the level.
     * If in the future, it's possible to set Tux start pos, this case
     * should contemplate that. */
    else if (start_position.x > 0 && start_position.x < screen->w &&
             start_position.y > 0 && start_position.y < screen->h)
      activate(LEFT);
    }
  else
    {
    if(base.x + base.width < scroll_x - X_OFFSCREEN_DISTANCE*4
      || base.x > scroll_x + screen->w + X_OFFSCREEN_DISTANCE*4
      || base.y + base.height < scroll_y - Y_OFFSCREEN_DISTANCE*4
      || base.y > scroll_y + screen->h + Y_OFFSCREEN_DISTANCE*4)
      {
      seen = false;
      if(dying != DYING_NOT)
        remove_me();
      }
    }

  if(!seen)
    return;

  switch (kind)
    {
    case BAD_MRICEBLOCK:
      action_mriceblock(elapsed_time);
      break;
  
    case BAD_JUMPY:
      action_jumpy(elapsed_time);
      break;

    case BAD_MRBOMB:
      action_mrbomb(elapsed_time);
      break;
    
    case BAD_BOMB:
      action_bomb(elapsed_time);
      break;

    case BAD_STALACTITE:
      action_stalactite(elapsed_time);
      break;

    case BAD_FLAME:
      action_flame(elapsed_time);
      break;

    case BAD_FISH:
    case BAD_FLAMEFISH:
      action_fish(elapsed_time);
      break;

    case BAD_BOUNCINGSNOWBALL:
      action_bouncingsnowball(elapsed_time);
      break;

    case BAD_FLYINGSNOWBALL:
      action_flyingsnowball(elapsed_time);
      break;

    case BAD_SPIKY:
      action_spiky(elapsed_time);
      break;

    case BAD_SNOWBALL:
      action_snowball(elapsed_time);
      break;

    case BAD_WINGLING:
      action_wingling(elapsed_time);
      break;

    case BAD_WALKINGTREE:
      action_walkingtree(elapsed_time);
      break;

    default:
      break;
    }
}

void
BadGuy::draw(DrawingContext& context)
{
  if(!seen)
    return;

  if((dir == LEFT && action_left == "hide") ||
     (dir == RIGHT && action_right == "hide"))
    return;

  if(dir == LEFT)
    specs->sprite->set_action(action_left);
  else  // if(dir == RIGHT)
    specs->sprite->set_action(action_right);

  if(dying == DYING_FALLING && physic.get_velocity_y() < 0)
    specs->sprite->draw(context, Vector(base.x, base.y), LAYER_FOREGROUNDTILES+1, VERTICAL_FLIP);
  else
    specs->sprite->draw(context, Vector(base.x, base.y), LAYER_OBJECTS);

  if(debug_mode)
    context.draw_filled_rect(Vector(base.x, base.y),
        Vector(base.width, base.height), Color(75,0,75, 150), LAYER_OBJECTS+1);
}

void
BadGuy::set_action(std::string left, std::string right)
{
  base.width = 32;
  base.height = 32;

  action_left = left;
  action_right = right;

#if 0
  if (1)
    {
      base.width = 32;
      base.height = 32;
    }
  else
    {
      // FIXME: Using the image size for the physics and collision is
      // a bad idea, since images should always overlap their physical
      // representation
      if(left != 0) {
        if(base.width == 0 && base.height == 0) {
          base.width  = left->get_width();
          base.height = left->get_height();
        } else if(base.width != left->get_width() || base.height != left->get_height()) {
          base.x -= (left->get_width() - base.width) / 2;
          base.y -= left->get_height() - base.height;
          base.width = left->get_width();
          base.height = left->get_height();
          old_base = base;
        }
      } else {
        base.width = base.height = 0;
      }
    }

  animation_offset = 0;
  sprite_left  = left;
  sprite_right = right;
#endif
}

void
BadGuy::bump()
{
  // these can't be bumped
  if(kind == BAD_FLAME || kind == BAD_BOMB || kind == BAD_FISH
      || kind == BAD_FLAMEFISH || kind == BAD_FLYINGSNOWBALL)
    return;

  physic.set_velocity_y(3);
  kill_me(25);
}

void
BadGuy::squish_me(Player* player)
{
  player->bounce(this);
    
  Sector::current()->add_score(Vector(base.x, base.y),
                              25 * player_status.score_multiplier);

  SoundManager::get()->play_sound(IDToSound(SND_SQUISH), get_pos(), Sector::current()->player->get_pos());
  player_status.score_multiplier++;

  dying = DYING_SQUISHED;
  timer.start(2000);
  physic.set_velocity(0, 0);
}

void
BadGuy::squish(Player* player)
{
  static const int MAX_ICEBLOCK_SQUICHES = 10;
    
  if(kind == BAD_MRBOMB) {
    // mrbomb transforms into a bomb now
    explode(false);
    
    player->bounce(this);
    Sector::current()->add_score(Vector(base.x, base.y),
                                25 * player_status.score_multiplier);
    SoundManager::get()->play_sound(IDToSound(SND_SQUISH), get_pos(), Sector::current()->player->get_pos());

    player_status.score_multiplier++;
    return;

  } else if (kind == BAD_MRICEBLOCK) {
    if (mode == NORMAL || mode == KICK)
      {
        /* Flatten! */
        SoundManager::get()->play_sound(IDToSound(SND_STOMP), get_pos(), Sector::current()->player->get_pos());
        mode = FLAT;
        set_action("flat-left", "flat-right");
        physic.set_velocity_x(0);

        timer.start(4000);
      } else if (mode == FLAT) {
        /* Kick! */
        SoundManager::get()->play_sound(IDToSound(SND_KICK), this, Sector::current()->player->get_pos());

        if (player->base.x < base.x + (base.width/2)) {
          physic.set_velocity_x(5);
          dir = RIGHT;
        } else {
          physic.set_velocity_x(-5);
          dir = LEFT;
        }

        mode = KICK;
        player->kick_timer.start(KICKING_TIME);
        set_action("flat-left", "flat-right");
      }

    player->bounce(this);

    player_status.score_multiplier++;

    // check for maximum number of squishes
    squishcount++;
    if(squishcount >= MAX_ICEBLOCK_SQUICHES) {
      kill_me(50);
      return;
    }
    
    return;
  } else if(kind == BAD_FISH || kind == BAD_FLAMEFISH) {
    // fish can only be killed when falling down
    if(physic.get_velocity_y() >= 0)
      return;
      
    player->bounce(this);
	      
    Sector::current()->add_score(Vector(base.x, base.y),
                                25 * player_status.score_multiplier);

    player_status.score_multiplier++;
     
    // simply remove the fish...
    remove_me();
    return;
  } else if(kind == BAD_BOUNCINGSNOWBALL) {
    squish_me(player);
    set_action("squished", "squished");
    return;
  } else if(kind == BAD_FLYINGSNOWBALL) {
    squish_me(player);
    set_action("squished-left", "squished-right");
    return;
  } else if(kind == BAD_SNOWBALL) {
    squish_me(player);
    set_action("squished-left", "squished-right");
    return;
  } else if(kind == BAD_WINGLING) {
    squish_me(player);
    set_action("left", "right");
  } else if(kind == BAD_WALKINGTREE) {
    if (mode == BGM_BIG)
    {
      set_action("left-small", "left-small");
      physic.set_velocity_x(physic.get_velocity_x() * 2.0f);

      /* Move to the player's direction */
      if(dir != Sector::current()->player->dir)
        physic.set_velocity_x(-physic.get_velocity_x());
      dir = Sector::current()->player->dir;

      // XXX magic number: 66 is BGM_BIG height

      player->bounce(this);
      base.y += 66 - base.height;

      Sector::current()->add_score(Vector(base.x, base.y),
                                25 * player_status.score_multiplier);
      player_status.score_multiplier++;

      mode = BGM_SMALL;
    }
    else
      squish_me(player);
  }
}

void
BadGuy::kill_me(int score)
{
  if(kind == BAD_BOMB)
    return;

  if(mode != HELD)
    global_stats.add_points(BADGUYS_KILLED_STAT, 1);

  dying = DYING_FALLING;
  if(kind == BAD_MRICEBLOCK) {
    set_action("falling-left", "falling-right");
    if(mode == HELD) {
      mode = NORMAL;
      Player& tux = *Sector::current()->player;
      tux.holding_something = false;
    }
  }

  physic.enable_gravity(true);

  /* Gain some points: */
  if (score != 0)
    Sector::current()->add_score(Vector(base.x, base.y),
                                score * player_status.score_multiplier);

  /* Play death sound: */
  SoundManager::get()->play_sound(IDToSound(SND_FALL), this, Sector::current()->player->get_pos());
}

void
BadGuy::explode(bool right_way)
{
  BadGuy *badguy = Sector::current()->add_bad_guy(base.x, base.y, BAD_BOMB, true);
  if(right_way)
    {
    badguy->timer.start(0);
    badguy->mode = BOMB_TICKING;
    }
  badguy->dir = dir;

  remove_me();
}

void
BadGuy::collision(const MovingObject&, int)
{
  // later
}

void
BadGuy::collision(void *p_c_object, int c_object, CollisionType type)
{
  if(!seen)
    return;

  BadGuy* pbad_c    = NULL;
  Bullet* pbullet_c = NULL;

  if(type == COLLISION_BUMP) {
    bump();
    return;
  }

  if(type == COLLISION_SQUISH) {
    Player* player = static_cast<Player*>(p_c_object);
    squish(player);
    return;
  }

  /* COLLISION_NORMAL */
  switch (c_object)
    {
    case CO_BULLET:
      pbullet_c = (Bullet*) p_c_object;

      if(pbullet_c->kind == FIRE_BULLET)
        {
        if (kind != BAD_BOMB && kind != BAD_STALACTITE && kind != BAD_FLAME
            && kind != BAD_FLAMEFISH)
          kill_me(10);
        }
      else if(pbullet_c->kind == ICE_BULLET)
        {
        if(kind == BAD_FLAME || kind == BAD_FLAMEFISH)
          kill_me(10);
        else
          frozen_timer.start(FROZEN_TIME);
        }
      break;

    case CO_BADGUY:
      pbad_c = (BadGuy*) p_c_object;


      /* If we're a kicked mriceblock, kill [almost] any badguys we hit */
      if(kind == BAD_MRICEBLOCK && mode == KICK &&
         kind != BAD_FLAME && kind != BAD_BOMB && kind != BAD_STALACTITE)
        {
          pbad_c->kill_me(25);
        }

      // a held mriceblock kills the enemy too but falls to ground then
      else if(kind == BAD_MRICEBLOCK && mode == HELD)
        {
          pbad_c->kill_me(25);
          kill_me(0);
        }

      /* Kill badguys that run into exploding bomb */
      else if (kind == BAD_BOMB && dying == DYING_NOT)
      {
        if (pbad_c->kind == BAD_MRBOMB)
        {
          // mrbomb transforms into a bomb now
          pbad_c->explode(true);
          return;
        }
        else
        {
          pbad_c->kill_me(50);
        }
      }

      /* Kill any badguys that get hit by stalactite */
      else if (kind == BAD_STALACTITE && dying == DYING_NOT)
      {
        if (pbad_c->kind == BAD_MRBOMB)
        {
          // mrbomb transforms into a bomb now
          pbad_c->explode(false);
          return;
        }
        else
          pbad_c->kill_me(50);
      }

      /* When enemies run into eachother, make them change directions */
      else
      {
        // Wingling doesn't interact with other badguys
        if (pbad_c->kind == BAD_WINGLING || kind == BAD_WINGLING)
          break;

        // Jumpy, fish, flame, stalactites, wingling are exceptions
        if (pbad_c->kind == BAD_JUMPY || pbad_c->kind == BAD_FLAME
            || pbad_c->kind == BAD_STALACTITE || pbad_c->kind == BAD_FISH
            || pbad_c->kind == BAD_FLAMEFISH)
          break;

        // Bounce off of other badguy if we land on top of him
        if (base.y + base.height < pbad_c->base.y + pbad_c->base.height)
        {
          if (pbad_c->dir == LEFT)
          {
            dir = RIGHT;
            physic.set_velocity(fabsf(physic.get_velocity_x()), 2);
          }
          else if (pbad_c->dir == RIGHT)
          {
            dir = LEFT;
            physic.set_velocity(-fabsf(physic.get_velocity_x()), 2);
          }

          break;
        }
        else if (base.y + base.height > pbad_c->base.y + pbad_c->base.height)
          break;

        if (pbad_c->kind != BAD_FLAME)
          {
            if (dir == LEFT)
            {
              dir = RIGHT;
              physic.set_velocity_x(fabsf(physic.get_velocity_x()));

              // Put bad guys a part (or they get jammed)
              // only needed to do to one of them
              if (physic.get_velocity_x() != 0)
                base.x = pbad_c->base.x + pbad_c->base.width + 1;
            }
            else if (dir == RIGHT)
            {
              dir = LEFT;
              physic.set_velocity_x(-fabsf(physic.get_velocity_x()));
            }

          }
      }
      
      break;

    case CO_PLAYER:
      Player* player = static_cast<Player*>(p_c_object);
      /* Get kicked if were flat */
      if (mode == FLAT && !dying)
      {
        SoundManager::get()->play_sound(IDToSound(SND_KICK), this, Sector::current()->player->get_pos());

        // Hit from left side
        if (player->base.x < base.x) {
          physic.set_velocity_x(5);
          dir = RIGHT;
        }
        // Hit from right side
        else {
          physic.set_velocity_x(-5);
          dir = LEFT;
        }

        mode = KICK;
        player->kick_timer.start(KICKING_TIME);
        set_action("flat-left", "flat-right");
      }
      break;

    }
}

// EOF //
