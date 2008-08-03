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

#include <math.h>
#include "gameloop.h"
#include "globals.h"
#include "player.h"
#include "defines.h"
#include "scene.h"
#include "tile.h"
#include "sprite.h"
#include "screen.h"

#define AUTOSCROLL_DEAD_INTERVAL 300

Surface* tux_life;

Sprite* smalltux_gameover;
Sprite* smalltux_star;
Sprite* largetux_star;

PlayerSprite smalltux;
PlayerSprite largetux;
PlayerSprite firetux;

PlayerKeymap keymap;

PlayerKeymap::PlayerKeymap()
{
  keymap.jump  = SDLK_SPACE;
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
}

void
Player::init()
{
  Level* plevel = World::current()->get_level();

  holding_something = false;

  base.width = 32;
  base.height = 32;

  size = SMALL;
  got_coffee = false;

  base.x = plevel->start_pos_x;
  base.y = plevel->start_pos_y;
  base.xm = 0;
  base.ym = 0;
  previous_base = old_base = base;
  dir = RIGHT;
  old_dir = dir;
  duck = false;

  dying   = DYING_NOT;
  jumping = false;
  can_jump = true;

  frame_main = 0;
  frame_ = 0;
  
  player_input_init(&input);

  invincible_timer.init(true);
  skidding_timer.init(true);
  safe_timer.init(true);
  frame_timer.init(true);
  kick_timer.init(true);

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
  else
    return false;
}

void
Player::level_begin()
{
  base.x  = 100;
  base.y  = 170;
  base.xm = 0;
  base.ym = 0;
  previous_base = old_base = base;
  duck = false;

  dying = DYING_NOT;

  player_input_init(&input);

  invincible_timer.init(true);
  skidding_timer.init(true);
  safe_timer.init(true);
  frame_timer.init(true);

  physic.reset();
}

void
Player::action(double frame_ratio)
{
  bool jumped_in_solid = false;

  if (input.fire == UP)
    holding_something = false;

  /* Move tux: */
  previous_base = base;

  /* --- HANDLE TUX! --- */
  if(dying == DYING_NOT)
    handle_input();

  physic.apply(frame_ratio, base.x, base.y);

  if(dying == DYING_NOT) 
    {
      base_type target = base;

      collision_swept_object_map(&old_base, &base);

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
          base.x += frame_ratio * WALK_SPEED * (dir ? 1 : -1);
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
          player_status.score_multiplier = 1;
        }

      if(jumped_in_solid)
        {
          if (isbrick(base.x, base.y) ||
              isfullbox(base.x, base.y))
            {
              World::current()->trygrabdistro(base.x, base.y - 32,BOUNCE);
              World::current()->trybumpbadguy(base.x, base.y - 64);

              World::current()->trybreakbrick(base.x, base.y, size == SMALL, RIGHT);

              bumpbrick(base.x, base.y);
              World::current()->tryemptybox(base.x, base.y, RIGHT);
            }

          if (isbrick(base.x+ 31, base.y) ||
              isfullbox(base.x+ 31, base.y))
            {
              World::current()->trygrabdistro(base.x+ 31, base.y - 32,BOUNCE);
              World::current()->trybumpbadguy(base.x+ 31, base.y - 64);

              if(size == BIG)
                World::current()->trybreakbrick(base.x+ 31, base.y, size == SMALL, LEFT);

              bumpbrick(base.x+ 31, base.y);
              World::current()->tryemptybox(base.x+ 31, base.y, LEFT);
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

//(GP2X patch)	updateSound();

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
           issolid(base.x + base.width - 1, base.y + base.height)  );
}

bool
Player::under_solid()
{
  return ( issolid(base.x + base.width / 2, base.y) ||
           issolid(base.x + 1, base.y) ||
           issolid(base.x + base.width - 1, base.y)  );
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
#ifndef NOSOUND
#ifndef GP2X
          play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);
#else
	  play_chunk(SND_SKID);
#endif
#endif
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
  // Press jump key
  if(input.up == DOWN && can_jump)
    {
      if (on_ground())
        {
          // jump higher if we are running
          if (fabs(physic.get_velocity_x()) > MAX_WALK_XM)
            physic.set_velocity_y(5.8);
          else
            physic.set_velocity_y(5.2);

          --base.y;
          jumping = true;
          can_jump = false;
          if (size == SMALL) {
#ifndef NOSOUND
#ifndef GP2X
            play_sound(sounds[SND_JUMP], SOUND_CENTER_SPEAKER);
#else
	    play_chunk(SND_JUMP);
#endif
		}
          else {
#ifndef GP2X
            play_sound(sounds[SND_BIGJUMP], SOUND_CENTER_SPEAKER);
#else
	    play_chunk(SND_BIGJUMP);
#endif
#endif
          }
        }
    }
  // Let go of jump key
  else if(input.up == UP && jumping)
    {
      jumping = false;
      if(physic.get_velocity_y() > 0) {
        physic.set_velocity_y(0);
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
  if (input.up == DOWN || (input.up == UP && jumping))
    {
      handle_vertical_input();
    }

  /* Shoot! */

  if (input.fire == DOWN && input.old_fire == UP && got_coffee)
    {
      World::current()->add_bullet(base.x, base.y, physic.get_velocity_x(), dir);
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
  else if(input.down == UP && size == BIG && duck && physic.get_velocity_y() == 0 && on_ground())
    {
      duck = false;
      base.y -= 32;
      base.height = 64;
      // changing base size confuses collision otherwise
      old_base = previous_base = base;                        
    }
}

void
Player::grow()
{
  if(size == BIG)
    return;
  
  size = BIG;
  base.height = 64;
  base.y -= 32;

  old_base = previous_base = base;
}

void
Player::jump_of_badguy(BadGuy* badguy)
{
  if(input.up)
    physic.set_velocity_y(5.2);
  else
    physic.set_velocity_y(2.0);
  base.y = badguy->base.y - base.height-2;
}

void
Player::grabdistros()
{
  /* Grab distros: */
  if (!dying)
    {
      World::current()->trygrabdistro(base.x, base.y, NO_BOUNCE);
      World::current()->trygrabdistro(base.x+ 31, base.y, NO_BOUNCE);

      World::current()->trygrabdistro(base.x, base.y + base.height, NO_BOUNCE);
      World::current()->trygrabdistro(base.x+ 31, base.y + base.height, NO_BOUNCE);

      if(size == BIG)
        {
          World::current()->trygrabdistro(base.x, base.y + base.height / 2, NO_BOUNCE);
          World::current()->trygrabdistro(base.x+ 31, base.y + base.height / 2, NO_BOUNCE);
        }

    }

  /* Enough distros for a One-up? */
  if (player_status.distros >= DISTROS_LIFEUP)
    {
      player_status.distros = player_status.distros - DISTROS_LIFEUP;
      if(player_status.lives < MAX_LIVES)
        ++player_status.lives;
      /*We want to hear the sound even, if MAX_LIVES is reached*/
#ifndef NOSOUND
#ifndef GP2X
      play_sound(sounds[SND_LIFEUP], SOUND_CENTER_SPEAKER);
#else
      play_chunk(SND_LIFEUP);
#endif
#endif
    }
}

void
Player::draw()
{
  if (!safe_timer.started() || (global_frame_counter % 2) == 0)
    {
      if (dying == DYING_SQUISHED)
        {
          smalltux_gameover->draw(base.x - scroll_x, base.y);
        }
      else
        {
          PlayerSprite* sprite;
          
          if (size == SMALL)
            sprite = &smalltux;
          else if (got_coffee)
            sprite = &firetux;
          else
            sprite = &largetux;
          
          if (duck && size != SMALL)
            {
              if (dir == RIGHT)
                sprite->duck_right->draw(base.x - scroll_x, base.y);
              else 
                sprite->duck_left->draw(base.x - scroll_x, base.y);
            }
          else if (skidding_timer.started())
            {
              if (dir == RIGHT)
                sprite->skid_right->draw(base.x - scroll_x, base.y);
              else
                sprite->skid_left->draw(base.x - scroll_x, base.y); 
            }
          else if (kick_timer.started())
            {
              if (dir == RIGHT)
                sprite->kick_right->draw(base.x - scroll_x, base.y);
              else
                sprite->kick_left->draw(base.x - scroll_x, base.y); 
            }
          else if (physic.get_velocity_y() != 0)
            {
              if (dir == RIGHT)
                sprite->jump_right->draw(base.x - scroll_x, base.y);
              else
                sprite->jump_left->draw(base.x - scroll_x, base.y);                   
            }
          else
            {
              if (fabsf(physic.get_velocity_x()) < 1.0f) // standing
                {
                  if (dir == RIGHT)
                    sprite->stand_right->draw( base.x - scroll_x, base.y);
                  else
                    sprite->stand_left->draw( base.x - scroll_x, base.y);
                }
              else // moving
                {
                  if (dir == RIGHT)
                    sprite->walk_right->draw(base.x - scroll_x, base.y);
                  else
                    sprite->walk_left->draw(base.x - scroll_x, base.y);
                }
            }
                      
          // Draw arm overlay graphics when Tux is holding something
          if (holding_something && physic.get_velocity_y() == 0 && !duck)
            {
              if (dir == RIGHT)
                sprite->grab_right->draw(base.x - scroll_x, base.y);
              else
                sprite->grab_left->draw(base.x - scroll_x, base.y);
            }

          // Draw blinking star overlay
          if (invincible_timer.started() &&
             (invincible_timer.get_left() > TUX_INVINCIBLE_TIME_WARNING || global_frame_counter % 3))
            {
              if (size == SMALL || duck)
                smalltux_star->draw(base.x - scroll_x, base.y);
              else
                largetux_star->draw(base.x - scroll_x, base.y);
            }
        }
    }     
  
  if (debug_mode)
    fillrect(base.x - scroll_x, base.y, 
             base.width, base.height, 75,75,75, 150);
}

void
Player::collision(void* p_c_object, int c_object)
{
  BadGuy* pbad_c = NULL;

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
                {
                   pbad_c->dying = DYING_FALLING;
#ifndef NOSOUND
#ifndef GP2X
                   play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
#else
		   play_chunk(SND_FALL);
#endif
#endif
                   World::current()->add_score(pbad_c->base.x - scroll_x,
                                               pbad_c->base.y,
                                               25 * player_status.score_multiplier);
                }
            }
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
    default:
      break;
    }

}

/* Kill Player! */

void
Player::kill(HurtMode mode)
{
#ifndef NOSOUND
#ifndef GP2X
  play_sound(sounds[SND_HURT], SOUND_CENTER_SPEAKER);
#else
  play_chunk(SND_HURT);
  updateSound();
#endif
#endif

  physic.set_velocity_x(0);

  if (mode == SHRINK && size == BIG)
    {
      if (got_coffee)
        {
          got_coffee = false;
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
      if(dying != DYING_SQUISHED)
      --player_status.lives;
      dying = DYING_SQUISHED;
    }
}

void
Player::is_dying()
{
  remove_powerups();
  dying = DYING_NOT;
}

bool Player::is_dead()
{
#ifndef RES320X240
  if(base.y > screen->h || base.x < scroll_x - AUTOSCROLL_DEAD_INTERVAL)  // last condition can happen in auto-scrolling
#else
  if(base.y > 640 || base.x < scroll_x - AUTOSCROLL_DEAD_INTERVAL)  // last condition can happen in auto-scrolling
#endif
    return true;
  else
    return false;
}

/* Remove Tux's power ups */
void
Player::remove_powerups()
{
  got_coffee = false;
  size = SMALL;
  base.height = 32;
}

void
Player::check_bounds(bool back_scrolling, bool hor_autoscroll)
{
  /* Keep tux in bounds: */
  if (base.x < 0)
    { // Lock Tux to the size of the level, so that he doesn't fall of
      // on the left side
      base.x = 0;
    }

  /* Keep in-bounds, vertically: */
#ifndef RES320X240
  if (base.y > screen->h)
#else
  if (base.y > 640)
#endif
    {
      kill(KILL);
#ifndef NOSOUND
#ifdef GP2X    
      float wait=SDL_GetTicks()+800;
      while ( wait > SDL_GetTicks()) {
         updateSound();
      }
#endif
#endif

    }

  if(base.x < scroll_x && (!back_scrolling || hor_autoscroll))  // can happen if back scrolling is disabled
    base.x = scroll_x;

  if(hor_autoscroll)
    {
    if(base.x == scroll_x)
      if((issolid(base.x+32, base.y) || (size != SMALL && !duck && issolid(base.x+32, base.y+32))) && (dying == DYING_NOT))
        kill(KILL);

    if(base.x + base.width > scroll_x + screen->w)
      base.x = scroll_x + screen->w - base.width;
    }
    
}

// EOF //

