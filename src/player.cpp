//
// C Implementation: player/tux
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de> & Bill Kendrick, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <math.h>

#include "gameloop.h"
#include "globals.h"
#include "player.h"
#include "defines.h"
#include "scene.h"
#include "tile.h"
#include "sprite.h"
#include "screen.h"

Surface* tux_life;
std::vector<Surface*> tux_right;
std::vector<Surface*> tux_left;
Surface* smalltux_jump_left;
Surface* smalltux_jump_right;
Surface* smalltux_stand_left;
Surface* smalltux_stand_right;

Sprite* bigtux_right;
Sprite* bigtux_left;
Sprite* bigtux_right_jump;
Sprite* bigtux_left_jump;
Surface* ducktux_right;
Surface* ducktux_left;
Surface* skidtux_right;
Surface* skidtux_left;
Surface* firetux_right[3];
Surface* firetux_left[3];
Surface* bigfiretux_right[3];
Surface* bigfiretux_left[3];
Surface* bigfiretux_right_jump;
Surface* bigfiretux_left_jump;
Surface* duckfiretux_right;
Surface* duckfiretux_left;
Surface* skidfiretux_right;
Surface* skidfiretux_left;
Surface* cape_right[2];
Surface* cape_left[2];
Surface* bigcape_right[2];
Surface* bigcape_left[2];

void player_input_init(player_input_type* pplayer_input)
{
  pplayer_input->down = UP;
  pplayer_input->fire = UP;
  pplayer_input->left = UP;
  pplayer_input->old_fire = UP;
  pplayer_input->right = UP;
  pplayer_input->up = UP;
}

void
Player::init()
{
  base.width = 32;
  base.height = 32;

  size = SMALL;
  got_coffee = false;

  // FIXME: Make the start position configurable via the levelfile
  base.x = 100;
  base.y = 170;
  base.xm = 0;
  base.ym = 0;
  previous_base = old_base = base;
  dir = RIGHT;
  duck = false;

  dying   = DYING_NOT;
  jumping = false;

  frame_main = 0;
  frame_ = 0;
  
  player_input_init(&input);

  keymap.jump  = SDLK_UP;
  keymap.duck  = SDLK_DOWN;
  keymap.left  = SDLK_LEFT;
  keymap.right = SDLK_RIGHT;
  keymap.fire  = SDLK_LCTRL;

  invincible_timer.init(true);
  skidding_timer.init(true);
  safe_timer.init(true);
  frame_timer.init(true);

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

  /* --- HANDLE TUX! --- */

  if(dying == DYING_NOT)
    handle_input();

  /* Move tux: */
  previous_base = base;

  physic.apply(frame_ratio, base.x, base.y);
  if(dying == DYING_NOT) {
      collision_swept_object_map(&old_base, &base);
      // special exception for cases where we're stuck under tiles after
      // being ducked. In this case we drift out
      if(!duck && on_ground() && old_base.x == base.x && old_base.y == base.y
              && collision_object_map(&base)) {
          base.x += frame_ratio * WALK_SPEED * (dir ? 1 : -1);
          previous_base = old_base = base;
      }
      keep_in_bounds();
  }

  if (dying == DYING_NOT)
    {
      /* Land: */


      if( !on_ground())
        {
          physic.enable_gravity(true);
          if(under_solid())
            {
              // fall down
              physic.set_velocity(physic.get_velocity_x(), 0);
              jumped_in_solid = true;
            }
        }
      else
        {
          /* Land: */
          if (physic.get_velocity_y() < 0)
            {
              base.y = (int)(((int)base.y / 32) * 32);
              physic.set_velocity(physic.get_velocity_x(), 0);
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

              World::current()->trybreakbrick(base.x, base.y, size == SMALL);

              bumpbrick(base.x, base.y);
              World::current()->tryemptybox(base.x, base.y, RIGHT);
            }

          if (isbrick(base.x+ 31, base.y) ||
              isfullbox(base.x+ 31, base.y))
            {
              World::current()->trygrabdistro(base.x+ 31, base.y - 32,BOUNCE);
              World::current()->trybumpbadguy(base.x+ 31, base.y - 64);

              if(size == BIG)
                World::current()->trybreakbrick(base.x+ 31, base.y, size == SMALL);

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


  /* ---- DONE HANDLING TUX! --- */

  /* Handle invincibility timer: */
  if (get_current_music() == HERRING_MUSIC && !invincible_timer.check())
    {
      /*
         no, we are no more invincible
         or we were not in invincible mode
         but are we in hurry ?
       */

      // FIXME: Move this to gamesession
      if (GameSession::current()->time_left.get_left() < TIME_WARNING)
        {
          /* yes, we are in hurry
             stop the herring_song, prepare to play the correct
             fast level_song !
           */
          set_current_music(HURRYUP_MUSIC);
        }
      else
        {
          set_current_music(LEVEL_MUSIC);
        }

      /* start playing it */
      play_current_music();
    }

  // check some timers
  skidding_timer.check();
  invincible_timer.check();
  safe_timer.check();
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
  if(!duck && input.left == DOWN && input.right == UP) {
      dir = LEFT;
      dirsign = -1;
  } else if(!duck && input.left == UP && input.right == DOWN) {
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
          play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);
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
  if(input.up == DOWN)
    {
      if (on_ground() && !duck)
        {
          // jump
          physic.set_velocity(physic.get_velocity_x(), 5.5);
          --base.y;
          jumping = true;
          if (size == SMALL)
            play_sound(sounds[SND_JUMP], SOUND_CENTER_SPEAKER);
          else
            play_sound(sounds[SND_BIGJUMP], SOUND_CENTER_SPEAKER);
        }
    }
  else if(input.up == UP && jumping)
    {
      jumping = false;
      if(physic.get_velocity_y() > 0) {
        physic.set_velocity(physic.get_velocity_x(), 0);
      }
    }
}

void
Player::handle_input()
{
  /* Handle horizontal movement: */
    handle_horizontal_input();

  /* Jump/jumping? */

  if ( input.up == DOWN || (input.up == UP && jumping))
    {
      handle_vertical_input();
    }

  /* Shoot! */

  if (input.fire == DOWN && input.old_fire == UP && got_coffee)
    {
      World::current()->add_bullet(base.x, base.y, physic.get_velocity_x(), dir);
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
  if (input.down == DOWN && size == BIG && !duck)
    {
      duck = true;
      base.height = 32;                             
      base.y += 32;
      // changing base size confuses collision otherwise
      old_base = previous_base = base;
    }
  else if(input.down == UP && size == BIG && duck)
    {
      duck = false;
      base.y -= 32;
      base.height = 64;
      old_base = previous_base = base;
    }
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
      play_sound(sounds[SND_LIFEUP], SOUND_CENTER_SPEAKER);
    }
}

void
Player::draw()
{
  if (!safe_timer.started() || (global_frame_counter % 2) == 0)
    {
      if (size == SMALL)
        {
          if (invincible_timer.started())
            {
              /* Draw cape: */

              if (dir == RIGHT)
                {
                 cape_right[global_frame_counter % 2]->draw(base.x- scroll_x, base.y);
                }
              else
                {
                  cape_left[global_frame_counter % 2]->draw(
                               base.x- scroll_x, base.y);
                }
            }


          if (!got_coffee)
            {
              if (physic.get_velocity_y() != 0)
                {
                  if (dir == RIGHT)
                    smalltux_jump_right->draw( base.x - scroll_x, base.y - 10);
                  else
                    smalltux_jump_left->draw( base.x - scroll_x, base.y - 10);                   
                }
              else
                {
                  if (fabsf(physic.get_velocity_x()) < 1.0f) // standing
                    {
                      if (dir == RIGHT)
                        smalltux_stand_right->draw( base.x - scroll_x, base.y - 9);
                      else
                        smalltux_stand_left->draw( base.x - scroll_x, base.y - 9);
                    }
                  else // moving
                    {
                      if (dir == RIGHT)
                        tux_right[(global_frame_counter/2) % tux_right.size()]->draw( 
                                     base.x - scroll_x, base.y - 9);
                      else
                        tux_left[(global_frame_counter/2) % tux_left.size()]->draw( 
                                     base.x - scroll_x, base.y - 9);
                    }
                }
            }
          else
            {
              /* Tux got coffee! */

              if (dir == RIGHT)
                {
                  firetux_right[frame_]->draw( base.x- scroll_x, base.y);
                }
              else
                {
                  firetux_left[frame_]->draw( base.x- scroll_x, base.y);
                }
            }
        }
      else
        {
          if (invincible_timer.started())
            {
              float capex = base.x + (base.width - bigcape_right[0]->w) / 2;
              capex -= scroll_x;
              float capey = base.y + (base.height - bigcape_right[0]->h) / 2;
                
              /* Draw cape (just not in ducked mode since that looks silly): */
              if (dir == RIGHT)
                {
                  bigcape_right[global_frame_counter % 2]->draw(
                          capex, capey);
                }
              else
                {
                  bigcape_left[global_frame_counter % 2]->draw(
                          capex, capey);
                }
            }

          if (!got_coffee)
            {
              if (!duck)
                {
                  if (!skidding_timer.started())
                    {
                      if (physic.get_velocity_y() == 0)
                        {
                          if (dir == RIGHT)
                            bigtux_right->draw(base.x- scroll_x - 8, base.y);
                          else
                              bigtux_left->draw(base.x- scroll_x - 8, base.y);
                        }
                      else
                        {
                          if (dir == RIGHT)
                            bigtux_right_jump->draw(base.x- scroll_x - 8, base.y);
                          else
                            bigtux_left_jump->draw(base.x- scroll_x - 8, base.y);
                        }
                    }
                  else
                    {
                      if (dir == RIGHT)
                        skidtux_right->draw(base.x- scroll_x - 8, base.y);
                      else
                        skidtux_left->draw(base.x- scroll_x - 8, base.y);
                    }
                }
              else
                {
                  if (dir == RIGHT)
                    {
                      ducktux_right->draw( base.x- scroll_x - 8, base.y - 16);
                    }
                  else
                    {
                      ducktux_left->draw( base.x- scroll_x - 8, base.y - 16);
                    }
                }
            }
          else
            {
              /* Tux has coffee! */
              if (!duck)
                {
                  if (!skidding_timer.started())
                    {
                      if (!jumping || physic.get_velocity_y() > 0)
                        {
                          if (dir == RIGHT)
                            {
                              bigfiretux_right[frame_]->draw(
                                           base.x- scroll_x - 8, base.y);
                            }
                          else
                            {
                              bigfiretux_left[frame_]->draw(
                                           base.x- scroll_x - 8, base.y);
                            }
                        }
                      else
                        {
                          if (dir == RIGHT)
                            {
                              bigfiretux_right_jump->draw(
                                           base.x- scroll_x - 8, base.y);
                            }
                          else
                            {
                              bigfiretux_left_jump->draw(
                                           base.x- scroll_x - 8, base.y);
                            }
                        }
                    }
                  else
                    {
                      if (dir == RIGHT)
                        {
                          skidfiretux_right->draw(
                                       base.x- scroll_x - 8, base.y);
                        }
                      else
                        {
                          skidfiretux_left->draw(
                                       base.x- scroll_x - 8, base.y);
                        }
                    }
                }
              else
                {
                  if (dir == RIGHT)
                    {
                      duckfiretux_right->draw( base.x- scroll_x - 8, base.y - 16);
                    }
                  else
                    {
                      duckfiretux_left->draw( base.x- scroll_x - 8, base.y - 16);
                    }
                }
            }
        }
    }

  if (debug_mode)
    fillrect(base.x - scroll_x, base.y, 32, 32, 75,75,75, 150);
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
          pbad_c->mode != HELD)
        {
          if (pbad_c->mode == FLAT && input.fire == DOWN)
            {
              pbad_c->mode = HELD;
              pbad_c->base.y-=8;
            }
          else if (pbad_c->mode == FLAT)
            {
              // Don't get hurt if we're kicking a flat badguy!
            }
          else if (pbad_c->mode == KICK)
            {
              /* Hurt if you get hit by kicked laptop: */
              if (!invincible_timer.started())
                {
                  kill(SHRINK);
                }
              else
                {
                   pbad_c->dying = DYING_FALLING;
                   play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
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
                  pbad_c->kill_me();
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
Player::kill(int mode)
{
  play_sound(sounds[SND_HURT], SOUND_CENTER_SPEAKER);

  physic.set_velocity(0, physic.get_velocity_y());

  if (mode == SHRINK && size == BIG)
    {
      if (got_coffee)
        got_coffee = false;

      size = SMALL;
      base.height = 32;
      duck = false;

      safe_timer.start(TUX_SAFE_TIME);
    }
  else
    {
      physic.enable_gravity(true);
      physic.set_acceleration(0, 0);
      physic.set_velocity(0, 7);
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
  if(base.y > screen->h)
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
Player::keep_in_bounds()
{
  Level* plevel = World::current()->get_level();

  /* Keep tux in bounds: */
  if (base.x < 0)
    { // Lock Tux to the size of the level, so that he doesn't fall of
      // on the left side
      base.x = 0;
    }
  else if (base.x < scroll_x)
    { 
      base.x = scroll_x;
    }

  /* Keep in-bounds, vertically: */
  if (base.y > screen->h)
    {
      kill(KILL);
    }

  int scroll_threshold = screen->w/2 - 80;
  if (debug_mode)
    {
      scroll_x += screen->w/2;
      // Backscrolling for debug mode
      if (scroll_x < base.x - 80)
        scroll_x = base.x - 80;
      else if (scroll_x > base.x + 80)
        scroll_x = base.x + 80;
      scroll_x -= screen->w/2;

      if(scroll_x < 0)
        scroll_x = 0;
    }
  else
    {
      if (base.x > scroll_threshold + scroll_x
          && scroll_x < ((World::current()->get_level()->width * 32) - screen->w))
        {
          // FIXME: Scrolling needs to be handled by a seperate View
          // class, doing it as a player huck is ugly
          
          // Scroll the screen in past center:
          scroll_x = base.x - scroll_threshold;
          
          // Lock the scrolling to the levelsize, so that we don't
          // scroll over the right border
          if (scroll_x > 32 * plevel->width - screen->w)
            scroll_x = 32 * plevel->width - screen->w;
        }
    }
}

// EOF //

