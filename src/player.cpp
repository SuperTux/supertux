//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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

#include <cmath>
#include <iostream>
#include <cassert>

#include "gameloop.h"
#include "globals.h"
#include "player.h"
#include "defines.h"
#include "scene.h"
#include "tile.h"
#include "sprite.h"
#include "sector.h"
#include "tilemap.h"
#include "camera.h"
#include "gameobjs.h"
#include "resources.h"
#include "interactive_object.h"
#include "screen/screen.h"

// behavior definitions:
#define TILES_FOR_BUTTJUMP 3
// animation times (in ms):
#define SHOOTING_TIME 320
#define STOMP_TIME 250
// others stuff:
#define AUTOSCROLL_DEAD_INTERVAL 300

// growing animation
Surface* growingtux_left[GROWING_FRAMES];
Surface* growingtux_right[GROWING_FRAMES];

Surface* tux_life;

Sprite* smalltux_gameover;
Sprite* smalltux_star;
Sprite* largetux_star;

PlayerSprite smalltux;
PlayerSprite largetux;
PlayerSprite icetux;
PlayerSprite firetux;

PlayerKeymap keymap;

PlayerKeymap::PlayerKeymap()
{
  keymap.jump  = SDLK_SPACE;
  keymap.activate = SDLK_UP;
  keymap.duck  = SDLK_DOWN;
  keymap.left  = SDLK_LEFT;
  keymap.right = SDLK_RIGHT;
  keymap.fire  = SDLK_LCTRL;
}

void player_input_init(player_input_type* pplayer_input)
{
  pplayer_input->down = UP;
  pplayer_input->fire = UP;
  pplayer_input->left = UP;
  pplayer_input->old_fire = UP;
  pplayer_input->right = UP;
  pplayer_input->up = UP;
  pplayer_input->old_up = UP;
  pplayer_input->activate = UP;
}

Player::Player()
{
  init();
}

Player::~Player()
{
}

void
Player::init()
{
  holding_something = false;

  base.width = 32;
  base.height = 32;

  size = SMALL;
  got_power = NONE_POWER;

  base.x = 0;
  base.y = 0;
  previous_base = old_base = base;
  dir = RIGHT;
  old_dir = dir;
  duck = false;
  dead = false;

  dying   = DYING_NOT;
  last_ground_y = 0;
  fall_mode = ON_GROUND;
  jumping = false;
  can_jump = true;
  butt_jump = false;
  
  stomp_pos = Vector(0,0);
  frame_main = 0;
  frame_ = 0;

  player_input_init(&input);

  invincible_timer.init(true);
  skidding_timer.init(true);
  safe_timer.init(true);
  frame_timer.init(true);
  kick_timer.init(true);
  shooting_timer.init(true);
  growing_timer.init(true);

  physic.reset();
}

int
Player::key_event(SDLKey key, int state)
{
  if(key == keymap.right)
    {
      input.right = state;
      return true;
    }
  else if(key == keymap.left)
    {
      input.left = state;
      return true;
    }
  else if(key == keymap.jump)
    {
      input.up = state;
      return true;
    }
  else if(key == keymap.duck)
    {
      input.down = state;
      return true;
    }
  else if(key == keymap.fire)
    {
      if (state == UP)
        input.old_fire = UP;
      input.fire = state;
      return true;
    }
  else if(key == keymap.activate)
    {
      input.activate = state;

      if(state == DOWN) {
        /** check for interactive objects */
        for(Sector::InteractiveObjects::iterator i 
            = Sector::current()->interactive_objects.begin();
            i != Sector::current()->interactive_objects.end(); ++i) {
          if(rectcollision(base, (*i)->get_area())) {
            (*i)->interaction(INTERACTION_ACTIVATE);
          }
        }
      }
      
      return true;
    }
  else
    return false;
}

void
Player::level_begin()
{
  base.x  = 100;
  base.y  = 170;
  previous_base = old_base = base;
  duck = false;

  dying = DYING_NOT;

  player_input_init(&input);

  invincible_timer.init(true);
  skidding_timer.init(true);
  safe_timer.init(true);
  frame_timer.init(true);
  growing_timer.init(true);

  physic.reset();
}

void
Player::action(float elapsed_time)
{
  bool jumped_in_solid = false;

  if(dying && !dying_timer.check()) {
    dead = true;
    return;
  }

  if (input.fire == UP)
    holding_something = false;

  /* Move tux: */
  previous_base = base;

  /* --- HANDLE TUX! --- */
  if(dying == DYING_NOT)
    handle_input();

  physic.apply(elapsed_time, base.x, base.y);

  if(dying == DYING_NOT) 
    {
      base_type target = base;

      collision_swept_object_map(&old_base, &base);

      if ((!invincible_timer.started() && !safe_timer.started())
          && (isspike(base.x, base.y) || isspike(base.x + base.width, base.y)
          ||  isspike(base.x, base.y + base.height)
          ||  isspike(base.x + base.width, base.y + base.height)))
      {
         kill(SHRINK);
      }

      // Don't accelerate Tux if he is running against a wall
      if (target.x != base.x)
        {
          physic.set_velocity_x(0);
        }

      // special exception for cases where we're stuck under tiles after
      // being ducked. In this case we drift out
      if(!duck && on_ground() && old_base.x == base.x && old_base.y == base.y
         && collision_object_map(base))
        {
          base.x += elapsed_time * WALK_SPEED * (dir ? 1: -1);
          previous_base = old_base = base;
        }

      // Land:
      if (!on_ground())
        {
          physic.enable_gravity(true);
          if(under_solid())
            {
              // fall down
              physic.set_velocity_y(0);
              jumped_in_solid = true;
              jumping = false;
            }
        }
      else
        {
          /* Land: */
          if (physic.get_velocity_y() < 0)
            {
              base.y = (int)(((int)base.y / 32) * 32);
              physic.set_velocity_y(0);
            }

          physic.enable_gravity(false);
          /* Reset score multiplier (for multi-hits): */
          if (!invincible_timer.started())
            player_status.score_multiplier = 1;
        }

      if(jumped_in_solid)
        {
          if (isbrick(base.x, base.y) ||
              isfullbox(base.x, base.y))
            {
	      Sector::current()->trygrabdistro(
                  Vector(base.x, base.y - 32), BOUNCE);
              Sector::current()->trybumpbadguy(Vector(base.x, base.y - 64));

              Sector::current()->trybreakbrick(
                  Vector(base.x, base.y), size == SMALL);

              bumpbrick(base.x, base.y);
              Sector::current()->tryemptybox(Vector(base.x, base.y), RIGHT);
            }

          if (isbrick(base.x+ 31, base.y) ||
              isfullbox(base.x+ 31, base.y))
            {
	      Sector::current()->trygrabdistro(
                  Vector(base.x+ 31, base.y - 32), BOUNCE);
              Sector::current()->trybumpbadguy(Vector(base.x+ 31, base.y - 64));

              if(size == BIG)
                Sector::current()->trybreakbrick(
                    Vector(base.x+ 31, base.y), size == SMALL);

              bumpbrick(base.x+ 31, base.y);
              Sector::current()->tryemptybox(Vector(base.x+ 31, base.y), LEFT);
            }
        }

      grabdistros();

      if (jumped_in_solid)
        {
          ++base.y;
          ++old_base.y;
          if(on_ground())
            {
              /* Make sure jumping is off. */
              jumping = false;
            }
        }
    }

  /* ---- DONE HANDLING TUX! --- */

  // check some timers
  skidding_timer.check();
  invincible_timer.check();
  safe_timer.check();
  kick_timer.check();
}

bool
Player::on_ground()
{
  return ( issolid(base.x + base.width / 2, base.y + base.height) ||
           issolid(base.x + 1, base.y + base.height) ||
           issolid(base.x + base.width - 1, base.y + base.height));
}

bool
Player::under_solid()
{
  return ( issolid(base.x + base.width / 2, base.y) ||
           issolid(base.x + 1, base.y) ||
           issolid(base.x + base.width - 1, base.y)  );
}

bool
Player::tiles_on_air(int tiles)
{
  for(int t = 0; t != tiles; t++)
     {
     if(issolid(base.x + base.width / 2, base.y + base.height + (tiles*32)) ||
         issolid(base.x + 1, base.y + base.height + (tiles*32)) ||
         issolid(base.x + base.width - 1, base.y + base.height + (tiles*32)))
       return false;
     }
  return true;
}

void
Player::handle_horizontal_input()
{
  float vx = physic.get_velocity_x();
  float vy = physic.get_velocity_y();
  float ax = physic.get_acceleration_x();
  float ay = physic.get_acceleration_y();

  float dirsign = 0;
  if(input.left == DOWN && input.right == UP && (!duck || physic.get_velocity_y() != 0)) {
      old_dir = dir;
      dir = LEFT;
      dirsign = -1;
  } else if(input.left == UP && input.right == DOWN && (!duck || physic.get_velocity_y() != 0)) {
      old_dir = dir;
      dir = RIGHT;
      dirsign = 1;
  }

  if (input.fire == UP) {
      ax = dirsign * WALK_ACCELERATION_X;
      // limit speed
      if(vx >= MAX_WALK_XM && dirsign > 0) {
        vx = MAX_WALK_XM;
        ax = 0;
      } else if(vx <= -MAX_WALK_XM && dirsign < 0) {
        vx = -MAX_WALK_XM;
        ax = 0;
      }
  } else {
      ax = dirsign * RUN_ACCELERATION_X;
      // limit speed
      if(vx >= MAX_RUN_XM && dirsign > 0) {
        vx = MAX_RUN_XM;
        ax = 0;
      } else if(vx <= -MAX_RUN_XM && dirsign < 0) {
        vx = -MAX_RUN_XM;
        ax = 0;
      }
  }

  // we can reach WALK_SPEED without any acceleration
  if(dirsign != 0 && fabs(vx) < WALK_SPEED) {
    vx = dirsign * WALK_SPEED;
  }

  // changing directions?
  if(on_ground() && ((vx < 0 && dirsign >0) || (vx>0 && dirsign<0))) {
      if(fabs(vx)>SKID_XM && !skidding_timer.check()) {
          skidding_timer.start(SKID_TIME);
          sound_manager->play_sound(sounds[SND_SKID]);
          ax *= 2.5;
      } else {
          ax *= 2;
      }
  }

  // we get slower when not pressing any keys
  if(dirsign == 0) {
      if(fabs(vx) < WALK_SPEED) {
          vx = 0;
          ax = 0;
      } else if(vx < 0) {
          ax = WALK_ACCELERATION_X * 1.5;
      } else {
          ax = WALK_ACCELERATION_X * -1.5;
      }
  }

  // if we're on ice slow down acceleration or deceleration
  if (isice(base.x, base.y + base.height))
  {
    /* the acceleration/deceleration rate on ice is inversely proportional to
     * the current velocity.
     */

    // increasing 1 will increase acceleration/deceleration rate
    // decreasing 1 will decrease acceleration/deceleration rate
    //  must stay above zero, though
    if (ax != 0) ax *= 1 / fabs(vx);
  }

  physic.set_velocity(vx, vy);
  physic.set_acceleration(ax, ay);
}

void
Player::handle_vertical_input()
{
  // set fall mode...
  if(on_ground()) {
    fall_mode = ON_GROUND;
    last_ground_y = base.y;
  } else {
    if(base.y > last_ground_y)
      fall_mode = FALLING;
    else if(fall_mode == ON_GROUND)
      fall_mode = JUMPING;
  }

  // Press jump key
  if(input.up == DOWN && can_jump && on_ground())
    {
      if(duck) { // only jump a little bit when in duck mode {
        physic.set_velocity_y(3);
      } else {
        // jump higher if we are running
        if (fabs(physic.get_velocity_x()) > MAX_WALK_XM)
          physic.set_velocity_y(5.8);
        else
          physic.set_velocity_y(5.2);
      }

      --base.y;
      jumping = true;
      can_jump = false;
      if (size == SMALL)
        sound_manager->play_sound(sounds[SND_JUMP]);
      else
        sound_manager->play_sound(sounds[SND_BIGJUMP]);
    }
  // Let go of jump key
  else if(input.up == UP && jumping && physic.get_velocity_y() > 0)
    {
      jumping = false;
      physic.set_velocity_y(0);
    }

   /* In case the player has pressed Down while in a certain range of air,
      enable butt jump action */
  if (input.down == DOWN && !butt_jump && !duck)
    if(tiles_on_air(TILES_FOR_BUTTJUMP) && jumping)
      butt_jump = true;

   /* When Down is not held anymore, disable butt jump */
  if(butt_jump && input.down == UP)
    butt_jump = false;

  // Do butt jump
  if (butt_jump && on_ground() && size == BIG)
  {
    
    if (duck) 
      stomp_pos = Vector(base.x - 32, base.y);
    else 
      stomp_pos = Vector(base.x - 32, base.y + 32);    
    stomp_timer.start(STOMP_TIME);
    
    butt_jump = false;

    // Break bricks beneath Tux
    if(Sector::current()->trybreakbrick(
          Vector(base.x + 1, base.y + base.height), false)
        || Sector::current()->trybreakbrick(
           Vector(base.x + base.width - 1, base.y + base.height), false))
    {
      physic.set_velocity_y(2);
      butt_jump = true;
    }

    // Kill nearby badguys
    std::vector<GameObject*> gameobjects = Sector::current()->gameobjects;
    for (std::vector<GameObject*>::iterator i = gameobjects.begin();
         i != gameobjects.end();
         i++)
    {
      BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
      if(badguy)
      {
        
	if (fabsf(base.x - badguy->base.x) < 150 &&
            fabsf(base.y - badguy->base.y) < 60 &&
            (issolid(badguy->base.x + 1, badguy->base.y + badguy->base.height) ||
              issolid(badguy->base.x + badguy->base.width - 1, badguy->base.y + badguy->base.height)))
          badguy->kill_me(25);
      }
    }
  }

  if ( (issolid(base.x + base.width / 2, base.y + base.height + 64) ||
        issolid(base.x + 1, base.y + base.height + 64) ||
        issolid(base.x + base.width - 1, base.y + base.height + 64))
       && jumping  == false
       && can_jump == false
       && input.up == DOWN
       && input.old_up == UP)
    {
      can_jump = true;
    }

  if(on_ground())   /* Make sure jumping is off. */
    jumping = false;

  input.old_up = input.up;
}

void
Player::handle_input()
{
  /* Handle horizontal movement: */
    handle_horizontal_input();

  /* Jump/jumping? */

  if (on_ground() && input.up == UP)
    can_jump = true;
  handle_vertical_input();

  /* Shoot! */
  if (input.fire == DOWN && input.old_fire == UP && got_power != NONE_POWER)
    {
      if(Sector::current()->add_bullet(Vector(base.x, base.y + (base.height/2)),
          physic.get_velocity_x(), dir))
        shooting_timer.start(SHOOTING_TIME);
      input.old_fire = DOWN;
    }

  /* tux animations: */
  if(!frame_timer.check())
    {
      frame_timer.start(25);
      if (input.right == UP && input.left == UP)
        {
          frame_main = 1;
          frame_ = 1;
        }
      else
        {
          if ((input.fire == DOWN && (global_frame_counter % 2) == 0) ||
              (global_frame_counter % 4) == 0)
            frame_main = (frame_main + 1) % 4;

          frame_ = frame_main;

          if (frame_ == 3)
            frame_ = 1;
        }
    }

  /* Duck! */
  if (input.down == DOWN && size == BIG && !duck && physic.get_velocity_y() == 0 && on_ground())
    {
      duck = true;
      base.height = 32;                             
      base.y += 32;
      // changing base size confuses collision otherwise
      old_base = previous_base = base;
    }
  else if(input.down == UP && size == BIG && duck)
    {
      // try if we can really unduck
      base.y -= 32;
      base.height = 64;
      // when unducking in air we need some space to do so
      if(on_ground() || !collision_object_map(base)) {
        duck = false;
        // changing base size confuses collision otherwise
        old_base = previous_base = base;                                
      } else {
        // undo the ducking changes
        base.y += 32;
        base.height = 32;
      }   
    }
}

void
Player::grow(bool animate)
{
  if(size == BIG)
    return;
  
  size = BIG;
  base.height = 64;
  base.y -= 32;

  if(animate)
    growing_timer.start(GROWING_TIME);

  old_base = previous_base = base;
}

void
Player::grabdistros()
{
  /* Grab distros: */
  if (!dying)
    {
      Sector::current()->trygrabdistro(Vector(base.x, base.y), NO_BOUNCE);
      Sector::current()->trygrabdistro(Vector(base.x+ 31, base.y), NO_BOUNCE);
      Sector::current()->trygrabdistro(
          Vector(base.x, base.y + base.height), NO_BOUNCE);
      Sector::current()->trygrabdistro(
          Vector(base.x+ 31, base.y + base.height), NO_BOUNCE);

      if(size == BIG)
        {
	  Sector::current()->trygrabdistro(
              Vector(base.x, base.y + base.height / 2), NO_BOUNCE);
          Sector::current()->trygrabdistro(
              Vector(base.x+ 31, base.y + base.height / 2), NO_BOUNCE);
        }

    }

  /* Enough distros for a One-up? */
  if (player_status.distros >= DISTROS_LIFEUP)
    {
      player_status.distros = player_status.distros - DISTROS_LIFEUP;
      if(player_status.lives < MAX_LIVES)
        ++player_status.lives;
      /*We want to hear the sound even, if MAX_LIVES is reached*/
      sound_manager->play_sound(sounds[SND_LIFEUP]);
    }
}

void
Player::draw(DrawingContext& context)
{
  PlayerSprite* sprite;
          
  if (size == SMALL)
    sprite = &smalltux;
  else if (got_power == FIRE_POWER)
    sprite = &firetux;
  else if (got_power == ICE_POWER)
    sprite = &icetux;
  else
    sprite = &largetux;

  int layer = LAYER_OBJECTS - 1;
  Vector pos = Vector(base.x, base.y);

  if (!safe_timer.started() || (global_frame_counter % 2) == 0)
    {
      if (dying == DYING_SQUISHED)
        {
          smalltux_gameover->draw(context, pos, LAYER_FOREGROUNDTILES+1);
        }
      else
        {
          if(growing_timer.check())
            {
              if (dir == RIGHT)
                context.draw_surface(growingtux_right[(growing_timer.get_gone() * GROWING_FRAMES) / GROWING_TIME], pos, layer);
              else 
                context.draw_surface(growingtux_left[(growing_timer.get_gone() * GROWING_FRAMES) / GROWING_TIME], pos, layer);
            }
          else if (duck && size != SMALL)
            {
              if (dir == RIGHT)
                sprite->duck_right->draw(context, pos, layer);
              else 
                sprite->duck_left->draw(context, pos, layer);
            }
          else if (skidding_timer.started())
            {
              if (dir == RIGHT)
                sprite->skid_right->draw(context, pos, layer);
              else
                sprite->skid_left->draw(context, pos, layer);
            }
          else if (kick_timer.started())
            {
              if (dir == RIGHT)
                sprite->kick_right->draw(context, pos, layer);
              else
                sprite->kick_left->draw(context, pos, layer);
            }
          else if (physic.get_velocity_y() != 0)
            {
              if (dir == RIGHT)
                sprite->jump_right->draw(context, pos, layer);
              else
                sprite->jump_left->draw(context, pos, layer);
            }
          else
            {
              if (fabsf(physic.get_velocity_x()) < 1.0f) // standing
                {
                  if (dir == RIGHT)
                    sprite->stand_right->draw(context, pos, layer);
                  else
                    sprite->stand_left->draw(context, pos, layer);
                }
              else // moving
                {
                  if (dir == RIGHT)
                    sprite->walk_right->draw(context, pos, layer);
                  else
                    sprite->walk_left->draw(context, pos, layer);
                }
            }
        }
    }     

  // Draw arm overlay graphics when Tux is holding something
  if ((holding_something && physic.get_velocity_y() == 0) || shooting_timer.check() && !duck)
  {
    if (dir == RIGHT)
      sprite->grab_right->draw(context, pos, LAYER_OBJECTS + 1);
    else
      sprite->grab_left->draw(context, pos, LAYER_OBJECTS + 1);
  }
  
  // Draw stomp clouds when doing a butt jump
  if (stomp_timer.check())
      sprite->stomp->draw(context, stomp_pos, LAYER_OBJECTS + 1);

  // Draw blinking star overlay
  if (invincible_timer.started() &&
      (invincible_timer.get_left() > TUX_INVINCIBLE_TIME_WARNING || global_frame_counter % 3))
  {
    if (size == SMALL || duck)
      smalltux_star->draw(context, pos, LAYER_OBJECTS + 2);
    else
      largetux_star->draw(context, pos, LAYER_OBJECTS + 2);
  }
 
  if (debug_mode)
    context.draw_filled_rect(Vector(base.x, base.y),
        Vector(base.width, base.height), Color(75,75,75, 150), LAYER_OBJECTS+1);
}

void
Player::collision(const MovingObject& other, int collision_type)
{
  (void) other;
  (void) collision_type;
  // will be implemented later
}

void
Player::collision(void* p_c_object, int c_object)
{
  BadGuy* pbad_c = NULL;
  Trampoline* ptramp_c = NULL;
  FlyingPlatform* pplatform_c = NULL;

  switch (c_object)
    {
    case CO_BADGUY:
      pbad_c = (BadGuy*) p_c_object;

     /* Hurt player if he touches a badguy */
      if (!pbad_c->dying && !dying &&
          !safe_timer.started() &&
          pbad_c->mode != BadGuy::HELD)
        {
          if (pbad_c->mode == BadGuy::FLAT && input.fire == DOWN
               && !holding_something)
            {
              holding_something = true;
              pbad_c->mode = BadGuy::HELD;
              pbad_c->base.y-=8;
            }
          else if (pbad_c->mode == BadGuy::FLAT)
            {
              // Don't get hurt if we're kicking a flat badguy!
            }
          else if (pbad_c->mode == BadGuy::KICK)
            {
              /* Hurt if you get hit by kicked laptop: */
              if (!invincible_timer.started())
                {
                  kill(SHRINK);
                }
              else
                pbad_c->kill_me(20);
            }
          else if (pbad_c->frozen_timer.check() && (pbad_c->kind == BAD_MRBOMB
              || pbad_c->kind == BAD_JUMPY || pbad_c->kind == BAD_FISH
              || pbad_c->kind == BAD_SPIKY))
                pbad_c->kill_me(20);
          else
            {
              if (!invincible_timer.started())
                {
                  kill(SHRINK);
                }
              else
                {
                  pbad_c->kill_me(25);
                }
            }
          player_status.score_multiplier++;
        }
      break;

    case CO_TRAMPOLINE:
      ptramp_c = (Trampoline*) p_c_object;
      
      // Pick up trampoline
      if (ptramp_c->mode != Trampoline::M_HELD && input.fire == DOWN && !holding_something && on_ground())
      {
        holding_something = true;
        ptramp_c->mode = Trampoline::M_HELD;
        ptramp_c->base.y -= 8;
      }
      // Set down trampoline
      else if (ptramp_c->mode == Trampoline::M_HELD && input.fire != DOWN)
      {
        holding_something = false;
        ptramp_c->mode = Trampoline::M_NORMAL;
        ptramp_c->base.y += 8;
        ptramp_c->physic.set_velocity(physic.get_velocity_x(), physic.get_velocity_y());

        //if (dir == RIGHT)
        //  ptramp_c->base.x = base.x + base.width+1;
        //else /* LEFT */
        //  ptramp_c->base.x = base.x - base.width-1;
      }
/*
      // Don't let tux walk through trampoline
      else if (ptramp_c->mode != Trampoline::M_HELD && on_ground())
      {
        if (physic.get_velocity_x() > 0) // RIGHT
        {
          physic.set_velocity_x(0);
          base.x = ptramp_c->base.x - base.width;
        }
        else if (physic.get_velocity_x() < 0) // LEFT
        {
          physic.set_velocity_x(0);
          base.x = ptramp_c->base.x + ptramp_c->base.width;
        }
      }
*/
      break;
    case CO_FLYING_PLATFORM:
      pplatform_c = (FlyingPlatform*) p_c_object;
      
      base.y = pplatform_c->base.y - base.height;
      physic.set_velocity_x(pplatform_c->get_vel_x());
      
      physic.enable_gravity(false);
      can_jump = true;
      fall_mode = ON_GROUND;
      break;

    default:
      break;
    }

}

/* Kill Player! */

void
Player::kill(HurtMode mode)
{
  if(dying)
    return;
  
  sound_manager->play_sound(sounds[SND_HURT]);

  physic.set_velocity_x(0);

  if (mode == SHRINK && size == BIG)
    {
      if (got_power != NONE_POWER)
        {
          got_power = NONE_POWER;
        }
      else
        {
          size = SMALL;
          base.height = 32;
          duck = false;
        }
      safe_timer.start(TUX_SAFE_TIME);
    }
  else
    {
      physic.enable_gravity(true);
      physic.set_acceleration(0, 0);
      physic.set_velocity(0, 7);
      --player_status.lives;
      dying = DYING_SQUISHED;
      dying_timer.start(3000);
    }
}

/* Remove Tux's power ups */
void
Player::remove_powerups()
{
  got_power = NONE_POWER;
  size = SMALL;
  base.height = 32;
}

void
Player::move(const Vector& vector)
{
  base.x = vector.x;
  base.y = vector.y;
  old_base = previous_base = base;
}

void
Player::check_bounds(Camera* camera)
{
  /* Keep tux in bounds: */
  if (base.x < 0)
    { // Lock Tux to the size of the level, so that he doesn't fall of
      // on the left side
      base.x = 0;
    }

  /* Keep in-bounds, vertically: */
  if (base.y > Sector::current()->solids->get_height() * 32)
    {
      kill(KILL);
      return;
    }

  bool adjust = false;
  // can happen if back scrolling is disabled
  if(base.x < camera->get_translation().x) {
    base.x = camera->get_translation().x;
    adjust = true;
  }
  if(base.x >= camera->get_translation().x + screen->w - base.width) {
    base.x = camera->get_translation().x + screen->w - base.width;
    adjust = true;
  }

  if(adjust) {
    // squished now?
    if(collision_object_map(base)) {
      kill(KILL);
      return;
    }
  }
}

