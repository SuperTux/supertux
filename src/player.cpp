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

#include "gameloop.h"
#include "globals.h"
#include "player.h"
#include "defines.h"
#include "scene.h"
#include "screen.h"

texture_type tux_life;
texture_type tux_right[3];
texture_type tux_left[3];
texture_type bigtux_right[3];
texture_type bigtux_left[3];
texture_type bigtux_right_jump;
texture_type bigtux_left_jump;
texture_type ducktux_right;
texture_type ducktux_left;
texture_type skidtux_right;
texture_type skidtux_left;
texture_type firetux_right[3];
texture_type firetux_left[3];
texture_type bigfiretux_right[3];
texture_type bigfiretux_left[3];
texture_type bigfiretux_right_jump;
texture_type bigfiretux_left_jump;
texture_type duckfiretux_right;
texture_type duckfiretux_left;
texture_type skidfiretux_right;
texture_type skidfiretux_left;
texture_type cape_right[2];
texture_type cape_left[2];
texture_type bigcape_right[2];
texture_type bigcape_left[2];

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

  base.x = 0;
  base.y = 240;
  base.xm = 0;
  base.ym = 0;
  old_base = base;
  dir = RIGHT;
  duck = false;

  dying   = DYING_NOT;
  jumping = false;

  frame_main = 0;
  frame_ = 0;
  lives = 3;
  score = 0;
  distros = 0;

  player_input_init(&input_);

  keymap.jump  = SDLK_UP;
  keymap.duck  = SDLK_DOWN;
  keymap.left  = SDLK_LEFT;
  keymap.right = SDLK_RIGHT;
  keymap.fire  = SDLK_LCTRL;

  timer_init(&invincible_timer,true);
  timer_init(&skidding_timer,true);
  timer_init(&safe_timer,true);
  timer_init(&frame_timer,true);
  physic_init(&hphysic);
  physic_init(&vphysic);
}

int
Player::key_event(SDLKey key, int state)
{
  if(key == keymap.right)
    {
      input_.right = state;
      return true;
    }
  else if(key == keymap.left)
    {
      input_.left = state;
      return true;
    }
  else if(key == keymap.jump)
    {
      input_.up = state;
      return true;
    }
  else if(key == keymap.duck)
    {
      input_.down = state;
      return true;
    }
  else if(key == keymap.fire)
    {
      input_.fire = state;
      return true;
    }
  else
    return false;
}

void
Player::level_begin()
{
  base.x = 0;
  base.y = 240;
  base.xm = 0;
  base.ym = 0;
  old_base = base;

  player_input_init(&input_);

  timer_init(&invincible_timer,true);
  timer_init(&skidding_timer,true);
  timer_init(&safe_timer,true);
  timer_init(&frame_timer,true);
  physic_init(&hphysic);
  physic_init(&vphysic);
}

void
Player::action()
{
  bool jumped_in_solid = false;

  /* --- HANDLE TUX! --- */

  input();

  /* Move tux: */

  previous_base = base;

  base.x += base.xm * frame_ratio;
  base.y += base.ym * frame_ratio;

  collision_swept_object_map(&old_base,&base);

  keep_in_bounds();

  /* Land: */

  if (!dying)
    {


      if( !on_ground())
        {
          if(under_solid())
            {
              physic_set_state(&vphysic,PH_VT);
              physic_set_start_vy(&vphysic,0);
              jumped_in_solid = true;
            }
          else
            {
              if(!physic_is_set(&vphysic))
                {
                  physic_set_state(&vphysic,PH_VT);
                  physic_set_start_vy(&vphysic,0);
                }
            }
          base.ym = physic_get_velocity(&vphysic);

        }
      else
        {

          /* Land: */

          if (base.ym > 0)
            {
              base.y = (int)(((int)base.y / 32) * 32);
              base.ym = 0;
            }

          physic_init(&vphysic);

          /* Reset score multiplier (for multi-hits): */

          score_multiplier = 1;
        }

      if(jumped_in_solid == true)
        {

          if (isbrick(base.x, base.y) ||
              isfullbox(base.x, base.y))
            {
              trygrabdistro(base.x, base.y - 32,BOUNCE);
              trybumpbadguy(base.x, base.y - 64);

              if(size == BIG)
                trybreakbrick(base.x, base.y);

              bumpbrick(base.x, base.y);
              tryemptybox(base.x, base.y, RIGHT);
            }

          if (isbrick(base.x+ 31, base.y) ||
              isfullbox(base.x+ 31, base.y))
            {
              trygrabdistro(base.x+ 31, base.y - 32,BOUNCE);
              trybumpbadguy(base.x+ 31, base.y - 64);

              if(size == BIG)
                trybreakbrick(base.x+ 31, base.y);

              bumpbrick(base.x+ 31, base.y);
              tryemptybox(base.x+ 31, base.y, LEFT);
            }


          if(size == SMALL)
            {
              /* Get a distro from a brick? */

              if (shape(base.x, base.y) == 'x' ||
                  shape(base.x, base.y) == 'y')
                {
                  add_bouncy_distro((((int)base.x)
                                     / 32) * 32,
                                    ((int)base.y / 32) * 32);
                  if (counting_distros == false)
                    {
                      counting_distros = true;
                      distro_counter = 100;
                    }

                  if (distro_counter <= 0)
                    level_change(&current_level,base.x,base.y - 1, 'a');

                  play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                  score = score + SCORE_DISTRO;
                  distros++;
                }
              else if (shape(base.x+ 31, base.y) == 'x' ||
                       shape(base.x+ 31, base.y) == 'y')
                {
                  add_bouncy_distro((((int)base.x + 31)
                                     / 32) * 32,
                                    ((int)base.y / 32) * 32);
                  if (counting_distros == false)
                    {
                      counting_distros = true;
                      distro_counter = 100;
                    }

                  if (distro_counter <= 0)
                    level_change(&current_level,base.x+ 31, base.y, 'a');

                  play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                  score = score + SCORE_DISTRO;
                  distros++;
                }
            }
        }

      grabdistros();
      if(jumped_in_solid == true)
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

  timer_check(&safe_timer);


  /* ---- DONE HANDLING TUX! --- */

  /* Handle invincibility timer: */


  if (get_current_music() == HERRING_MUSIC && !timer_check(&invincible_timer))
    {
      /*
         no, we are no more invincible
         or we were not in invincible mode
         but are we in hurry ?
       */


      if (timer_get_left(&time_left) < TIME_WARNING)
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

  /* Handle skidding: */

  timer_check(&skidding_timer);

  /* End of level? */

  if (base.x >= endpos && endpos != 0)
    {
      next_level = 1;
    }

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
Player::handle_horizontal_input(int newdir)
{
  if ((newdir ? (base.xm < -SKID_XM) : (base.xm > SKID_XM)) && !timer_started(&skidding_timer) &&
      dir == !newdir && on_ground())
    {
      timer_start(&skidding_timer, SKID_TIME);

      play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);

    }
  dir = newdir;


  if ((newdir ? (base.xm < 0) : (base.xm > 0)) && !isice(base.x, base.y + base.height) &&
      !timer_started(&skidding_timer))
    {
      base.xm = 0;
    }

  if (!duck)
    {
      if (dir == newdir)
        {
          /* Facing the direction we're jumping?  Go full-speed: */

          if (input_.fire == UP)
            {
              base.xm = base.xm + ( newdir ? WALK_SPEED : -WALK_SPEED) * frame_ratio;

              if(newdir)
                {
                  if (base.xm > MAX_WALK_XM)
                    base.xm = MAX_WALK_XM;
                }
              else
                {
                  if (base.xm < -MAX_WALK_XM)
                    base.xm = -MAX_WALK_XM;
                }
            }
          else if ( input_.fire == DOWN)
            {
              base.xm = base.xm + ( newdir ? RUN_SPEED : -RUN_SPEED) * frame_ratio;

              if(newdir)
                {
                  if (base.xm > MAX_RUN_XM)
                    base.xm = MAX_RUN_XM;
                }
              else
                {
                  if (base.xm < -MAX_RUN_XM)
                    base.xm = -MAX_RUN_XM;
                }
            }
          else
            {
              /* Not facing the direction we're jumping?
                 Go half-speed: */

              base.xm = base.xm + ( newdir ? (WALK_SPEED / 2) : -(WALK_SPEED / 2)) * frame_ratio;

              if(newdir)
                {
                  if (base.xm > MAX_WALK_XM / 2)
                    base.xm = MAX_WALK_XM / 2;
                }
              else
                {
                  if (base.xm < -MAX_WALK_XM / 2)
                    base.xm = -MAX_WALK_XM / 2;
                }
            }
        }

    }
}

void
Player::handle_vertical_input()
{
  if(input_.up == DOWN)
    {
      if (on_ground())
        {
          if(!physic_is_set(&vphysic))
            {
              physic_set_state(&vphysic,PH_VT);
              physic_set_start_vy(&vphysic,5.5);
              --base.y;
              jumping = true;
              if (size == SMALL)
                play_sound(sounds[SND_JUMP], SOUND_CENTER_SPEAKER);
              else
                play_sound(sounds[SND_BIGJUMP], SOUND_CENTER_SPEAKER);
            }
        }
    }
  else if(input_.up == UP && jumping == true)
    {
      if (on_ground())
        {
          physic_init(&vphysic);
          jumping = false;
        }
      else
        {
          jumping = false;
          if(physic_is_set(&vphysic))
            {
              if(physic_get_velocity(&vphysic) < 0.)
                {
                  physic_set_state(&vphysic,PH_VT);
                  physic_set_start_vy(&vphysic,0);
                }
            }
          else
            {
              if(!physic_is_set(&vphysic))
                {
                  physic_set_state(&vphysic,PH_VT);
                }
            }
        }
    }
}

void
Player::input()
{
  /* Handle key and joystick state: */

  if(duck == false)
    {
      if (input_.right == DOWN && input_.left == UP)
        {
          handle_horizontal_input(RIGHT);
        }
      else if (input_.left == DOWN && input_.right == UP)
        {
          handle_horizontal_input(LEFT);
        }
      else
        {
          if(base.xm > 0)
            {
              base.xm = (int)(base.xm - frame_ratio);
              if(base.xm < 0)
                base.xm = 0;
            }
          else if(base.xm < 0)
            {
              base.xm = (int)(base.xm + frame_ratio);
              if(base.xm > 0)
                base.xm = 0;
            }
        }
    }

  /* Jump/jumping? */

  if ( input_.up == DOWN || (input_.up == UP && jumping == true))
    {
      handle_vertical_input();
    }

  /* Shoot! */

  if (input_.fire == DOWN && input_.old_fire == UP && got_coffee)
    {
      add_bullet(base.x, base.y, base.xm, dir);
    }


  /* Duck! */

  if (input_.down == DOWN)
    {
      if (size == BIG && duck != true)
        {
          duck = true;
          base.height = 32;
          base.y += 32;
        }
    }
  else
    {
      if (size == BIG && duck == true)
        {
          /* Make sure we're not standing back up into a solid! */
          base.height = 64;
          base.y -= 32;

          if (!collision_object_map(&base) /*issolid(base.x + 16, base.y - 16)*/)
            {
              duck = false;
              base.height = 64;
              old_base.y -= 32;
              old_base.height = 64;
            }
          else
            {
              base.height = 32;
              base.y += 32;
            }
        }
      else
        {
          duck = false;
        }
    }

  /* (Tux): */

  if(!timer_check(&frame_timer))
    {
      timer_start(&frame_timer,25);
      if (input_.right == UP && input_.left == UP)
        {
          frame_main = 1;
          frame_ = 1;
        }
      else
        {
          if ((input_.fire == DOWN && (global_frame_counter % 2) == 0) ||
              (global_frame_counter % 4) == 0)
            frame_main = (frame_main + 1) % 4;

          frame_ = frame_main;

          if (frame_ == 3)
            frame_ = 1;
        }
    }

}

void
Player::grabdistros()
{
  /* Grab distros: */
  if (!dying)
    {
      trygrabdistro(base.x, base.y, NO_BOUNCE);
      trygrabdistro(base.x+ 31, base.y, NO_BOUNCE);

      trygrabdistro(base.x, base.y + base.height, NO_BOUNCE);
      trygrabdistro(base.x+ 31, base.y + base.height, NO_BOUNCE);

      if(size == BIG)
        {
          trygrabdistro(base.x, base.y + base.height / 2, NO_BOUNCE);
          trygrabdistro(base.x+ 31, base.y + base.height / 2, NO_BOUNCE);
        }

    }

  /* Enough distros for a One-up? */
  if (distros >= DISTROS_LIFEUP)
    {
      distros = distros - DISTROS_LIFEUP;
      if(lives < MAX_LIVES)
        lives++;
      /*We want to hear the sound even, if MAX_LIVES is reached*/
      play_sound(sounds[SND_LIFEUP], SOUND_CENTER_SPEAKER);
    }
}

void
Player::draw()
{
  if (!timer_started(&safe_timer) || (global_frame_counter % 2) == 0)
    {
      if (size == SMALL)
        {
          if (timer_started(&invincible_timer))
            {
              /* Draw cape: */

              if (dir == RIGHT)
                {
                  texture_draw(&cape_right[global_frame_counter % 2],
                               base.x- scroll_x, base.y);
                }
              else
                {
                  texture_draw(&cape_left[global_frame_counter % 2],
                               base.x- scroll_x, base.y);
                }
            }


          if (!got_coffee)
            {
              if (dir == RIGHT)
                {
                  texture_draw(&tux_right[frame_], base.x- scroll_x, base.y);
                }
              else
                {
                  texture_draw(&tux_left[frame_], base.x- scroll_x, base.y);
                }
            }
          else
            {
              /* Tux got coffee! */

              if (dir == RIGHT)
                {
                  texture_draw(&firetux_right[frame_], base.x- scroll_x, base.y);
                }
              else
                {
                  texture_draw(&firetux_left[frame_], base.x- scroll_x, base.y);
                }
            }
        }
      else
        {
          if (timer_started(&invincible_timer))
            {
              /* Draw cape: */
              if (dir == RIGHT)
                {
                  texture_draw(&bigcape_right[global_frame_counter % 2],
                               base.x- scroll_x - 8, base.y);
                }
              else
                {
                  texture_draw(&bigcape_left[global_frame_counter % 2],
                               base.x-scroll_x - 8, base.y);
                }
            }

          if (!got_coffee)
            {
              if (!duck)
                {
                  if (!timer_started(&skidding_timer))
                    {
                      if (!jumping || base.ym > 0)
                        {
                          if (dir == RIGHT)
                            {
                              texture_draw(&bigtux_right[frame_],
                                           base.x- scroll_x - 8, base.y);
                            }
                          else
                            {
                              texture_draw(&bigtux_left[frame_],
                                           base.x- scroll_x - 8, base.y);
                            }
                        }
                      else
                        {
                          if (dir == RIGHT)
                            {
                              texture_draw(&bigtux_right_jump,
                                           base.x- scroll_x - 8, base.y);
                            }
                          else
                            {
                              texture_draw(&bigtux_left_jump,
                                           base.x- scroll_x - 8, base.y);
                            }
                        }
                    }
                  else
                    {
                      if (dir == RIGHT)
                        {
                          texture_draw(&skidtux_right,
                                       base.x- scroll_x - 8, base.y);
                        }
                      else
                        {
                          texture_draw(&skidtux_left,
                                       base.x- scroll_x - 8, base.y);
                        }
                    }
                }
              else
                {
                  if (dir == RIGHT)
                    {
                      texture_draw(&ducktux_right, base.x- scroll_x - 8, base.y - 16);
                    }
                  else
                    {
                      texture_draw(&ducktux_left, base.x- scroll_x - 8, base.y - 16);
                    }
                }
            }
          else
            {
              /* Tux has coffee! */

              if (!duck)
                {
                  if (!timer_started(&skidding_timer))
                    {
                      if (!jumping || base.ym > 0)
                        {
                          if (dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right[frame_],
                                           base.x- scroll_x - 8, base.y);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left[frame_],
                                           base.x- scroll_x - 8, base.y);
                            }
                        }
                      else
                        {
                          if (dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right_jump,
                                           base.x- scroll_x - 8, base.y);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left_jump,
                                           base.x- scroll_x - 8, base.y);
                            }
                        }
                    }
                  else
                    {
                      if (dir == RIGHT)
                        {
                          texture_draw(&skidfiretux_right,
                                       base.x- scroll_x - 8, base.y);
                        }
                      else
                        {
                          texture_draw(&skidfiretux_left,
                                       base.x- scroll_x - 8, base.y);
                        }
                    }
                }
              else
                {
                  if (dir == RIGHT)
                    {
                      texture_draw(&duckfiretux_right, base.x- scroll_x - 8, base.y - 16);
                    }
                  else
                    {
                      texture_draw(&duckfiretux_left, base.x- scroll_x - 8, base.y - 16);
                    }
                }
            }
        }
    }
}

void
Player::collision(void* p_c_object, int c_object)
{
  BadGuy* pbad_c = NULL;

  switch (c_object)
    {
    case CO_BADGUY:
      pbad_c = (BadGuy*) p_c_object;
      /* Hurt the player if he just touched it: */

      if (!pbad_c->dying && !dying &&
          !timer_started(&safe_timer) &&
          pbad_c->mode != HELD)
        {
          if (pbad_c->mode == FLAT  && input_.fire != DOWN)
            {
              /* Kick: */

              pbad_c->mode = KICK;
              play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

              if (base.x < pbad_c->base.x + (pbad_c->base.width/2))
                {
                  pbad_c->dir = RIGHT;
                  pbad_c->base.x = pbad_c->base.x + 16;
                }
              else
                {
                  pbad_c->dir = LEFT;
                  pbad_c->base.x = pbad_c->base.x - 32;
                }

              timer_start(&pbad_c->timer,5000);
            }
          else if (pbad_c->mode == FLAT && input_.fire == DOWN)
            {
              pbad_c->mode = HELD;
              pbad_c->base.y-=8;
            }
          else if (pbad_c->mode == KICK)
            {
              if (base.y < pbad_c->base.y - 16 &&
                  timer_started(&pbad_c->timer))
                {
                  /* Step on (stop being kicked) */

                  pbad_c->mode = FLAT;
                  play_sound(sounds[SND_STOMP], SOUND_CENTER_SPEAKER);
                  timer_start(&pbad_c->timer, 10000);
                }
              else
                {
                  /* Hurt if you get hit by kicked laptop: */

                  if (timer_started(&pbad_c->timer))
                    {
                      if (!timer_started(&invincible_timer))
                        {
                          kill(SHRINK);
                        }
                      else
                        {
                          pbad_c->dying = DYING_FALLING;
                          play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
                          add_score(pbad_c->base.x - scroll_x,
                                    pbad_c->base.y,
                                    25 * score_multiplier);
                        }
                    }
                }
            }
          else
            {
              if (!timer_started(&invincible_timer ))
                {
                  kill(SHRINK);
                }
              else
                {
                  pbad_c->dying = DYING_FALLING;
                  play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
                  add_score(pbad_c->base.x - scroll_x,
                            pbad_c->base.y,
                            25 * score_multiplier);
                }
            }
          score_multiplier++;
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
  base.ym = -5;

  play_sound(sounds[SND_HURT], SOUND_CENTER_SPEAKER);

  if (dir == RIGHT)
    base.xm = -8;
  else if (dir == LEFT)
    base.xm = 8;

  if (mode == SHRINK && size == BIG)
    {
      if (got_coffee)
        got_coffee = false;

      size = SMALL;
      base.height = 32;

      timer_start(&safe_timer,TUX_SAFE_TIME);
    }
  else
    {
      dying = DYING_SQUISHED;
    }
}

void
Player::is_dying()
{
  base.ym = base.ym + gravity;

  /* He died :^( */

  --lives;
  remove_powerups();
  dying = DYING_NOT;
  
  level_begin();

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
  /* Keep tux in bounds: */
  if (base.x< 0)
    base.x= 0;
  else if(base.x< scroll_x)
    base.x= scroll_x;
  else if (base.x< 160 + scroll_x && scroll_x > 0 && debug_mode == true)
    {
      scroll_x = base.x- 160;
      /*base.x+= 160;*/

      if(scroll_x < 0)
        scroll_x = 0;

    }
  else if (base.x> screen->w / 2 + scroll_x && scroll_x < ((current_level.width * 32) - screen->w))
    {
      /* Scroll the screen in past center: */

      scroll_x = base.x- screen->w / 2;
      /*base.x= 320 + scroll_x;*/

      if (scroll_x > ((current_level.width * 32) - screen->w))
        scroll_x = ((current_level.width * 32) - screen->w);
    }
  else if (base.x> 608 + scroll_x)
    {
      /* ... unless there's no more to scroll! */

      /*base.x= 608 + scroll_x;*/
    }

  /* Keep in-bounds, vertically: */

  if (base.y > screen->h)
    {
      kill(KILL);
    }
}
