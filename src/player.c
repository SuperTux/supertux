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

texture_type tux_life,
tux_right[3],  tux_left[3],
bigtux_right[3],  bigtux_left[3],
bigtux_right_jump,  bigtux_left_jump,
ducktux_right,  ducktux_left,
skidtux_right,  skidtux_left,
firetux_right[3],  firetux_left[3],
bigfiretux_right[3],  bigfiretux_left[3],
bigfiretux_right_jump,  bigfiretux_left_jump,
duckfiretux_right,  duckfiretux_left,
skidfiretux_right,  skidfiretux_left,
cape_right[2],  cape_left[2],
bigcape_right[2],  bigcape_left[2];

void player_input_init(player_input_type* pplayer_input)
{
  pplayer_input->down = UP;
  pplayer_input->fire = UP;
  pplayer_input->left = UP;
  pplayer_input->old_fire = UP;
  pplayer_input->right = UP;
  pplayer_input->up = UP;
}

void player_init(player_type* pplayer)
{
  pplayer->base.width = 32;
  pplayer->base.height = 32;

  pplayer->size = SMALL;
  pplayer->got_coffee = NO;

  pplayer->base.x = 0;
  pplayer->base.y = 240;
  pplayer->base.xm = 0;
  pplayer->base.ym = 0;
  pplayer->old_base = pplayer->base;
  pplayer->dir = RIGHT;
  pplayer->duck = NO;

  pplayer->dying = NO;
  pplayer->jumping = NO;

  pplayer->frame_main = 0;
  pplayer->frame = 0;
  pplayer->lives = 3;
  pplayer->score = 0;
  pplayer->distros = 0;

  player_input_init(&pplayer->input);

  pplayer->keymap.jump = SDLK_UP;
  pplayer->keymap.duck = SDLK_DOWN;
  pplayer->keymap.left = SDLK_LEFT;
  pplayer->keymap.right = SDLK_RIGHT;
  pplayer->keymap.fire = SDLK_LCTRL;

  timer_init(&pplayer->invincible_timer,YES);
  timer_init(&pplayer->skidding_timer,YES);
  timer_init(&pplayer->safe_timer,YES);
  timer_init(&pplayer->frame_timer,YES);
  physic_init(&pplayer->hphysic);
  physic_init(&pplayer->vphysic);
}

int player_key_event(player_type* pplayer, SDLKey key, int state)
{
  if(key == pplayer->keymap.right)
    {
      pplayer->input.right = state;
      return YES;
    }
  else if( key == pplayer->keymap.left)
    {
      pplayer->input.left = state;
      return YES;
    }
  else if(key == pplayer->keymap.jump)
    {
      pplayer->input.up = state;
      return YES;
    }
  else if(key == pplayer->keymap.duck)
    {
      pplayer->input.down = state;
      return YES;
    }
  else if(key == pplayer->keymap.fire)
    {
      pplayer->input.fire = state;
      return YES;
    }
  else
    return NO;
}

void player_level_begin(player_type* pplayer)
{
  pplayer->base.x = 0;
  pplayer->base.y = 240;
  pplayer->base.xm = 0;
  pplayer->base.ym = 0;
  pplayer->old_base = pplayer->base;

  player_input_init(&pplayer->input);

  timer_init(&pplayer->invincible_timer,YES);
  timer_init(&pplayer->skidding_timer,YES);
  timer_init(&pplayer->safe_timer,YES);
  timer_init(&pplayer->frame_timer,YES);
  physic_init(&pplayer->hphysic);
  physic_init(&pplayer->vphysic);
}

void player_action(player_type* pplayer)
{
  int i, jumped_in_solid;
  jumped_in_solid = NO;

  /* --- HANDLE TUX! --- */

  player_input(pplayer);

  /* Move tux: */

  pplayer->previous_base = pplayer->base;

  pplayer->base.x += pplayer->base.xm * frame_ratio;
  pplayer->base.y += pplayer->base.ym * frame_ratio;

  collision_swept_object_map(&pplayer->old_base,&pplayer->base);

  player_keep_in_bounds(pplayer);

  /* Land: */

  if (!pplayer->dying)
    {


      if( !player_on_ground(pplayer))
        {
          if(player_under_solid(pplayer))
            {
              physic_set_state(&pplayer->vphysic,PH_VT);
              physic_set_start_vy(&pplayer->vphysic,0);
              jumped_in_solid = YES;
            }
          else
            {
              if(!physic_is_set(&pplayer->vphysic))
                {
                  physic_set_state(&pplayer->vphysic,PH_VT);
                  physic_set_start_vy(&pplayer->vphysic,0);
                }
            }
          pplayer->base.ym = physic_get_velocity(&pplayer->vphysic);

        }
      else
        {

          /* Land: */

          if (pplayer->base.ym > 0)
            {
              pplayer->base.y = (int)(((int)pplayer->base.y / 32) * 32);
              pplayer->base.ym = 0;
            }

          physic_init(&pplayer->vphysic);

          /* Reset score multiplier (for multi-hits): */

          score_multiplier = 1;
        }

      if(jumped_in_solid == YES)
        {

          if (isbrick(pplayer->base.x, pplayer->base.y) ||
              isfullbox(pplayer->base.x, pplayer->base.y))
            {
              trygrabdistro(pplayer->base.x, pplayer->base.y - 32,BOUNCE);
              trybumpbadguy(pplayer->base.x, pplayer->base.y - 64);

              if(pplayer->size == BIG)
                trybreakbrick(pplayer->base.x, pplayer->base.y);

              bumpbrick(pplayer->base.x, pplayer->base.y);
              tryemptybox(pplayer->base.x, pplayer->base.y, RIGHT);
            }

          if (isbrick(pplayer->base.x+ 31, pplayer->base.y) ||
              isfullbox(pplayer->base.x+ 31, pplayer->base.y))
            {
              trygrabdistro(pplayer->base.x+ 31, pplayer->base.y - 32,BOUNCE);
              trybumpbadguy(pplayer->base.x+ 31, pplayer->base.y - 64);

              if(pplayer->size == BIG)
                trybreakbrick(pplayer->base.x+ 31, pplayer->base.y);

              bumpbrick(pplayer->base.x+ 31, pplayer->base.y);
              tryemptybox(pplayer->base.x+ 31, pplayer->base.y, LEFT);
            }


          if(pplayer->size == SMALL)
            {
              /* Get a distro from a brick? */

              if (shape(pplayer->base.x, pplayer->base.y) == 'x' ||
                  shape(pplayer->base.x, pplayer->base.y) == 'y')
                {
                  add_bouncy_distro((((int)pplayer->base.x)
                                     / 32) * 32,
                                    ((int)pplayer->base.y / 32) * 32);
                  if (counting_distros == NO)
                    {
                      counting_distros = YES;
                      distro_counter = 100;
                    }

                  if (distro_counter <= 0)
                    level_change(&current_level,pplayer->base.x,pplayer->base.y - 1, 'a');

                  play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                  score = score + SCORE_DISTRO;
                  distros++;
                }
              else if (shape(pplayer->base.x+ 31, pplayer->base.y) == 'x' ||
                       shape(pplayer->base.x+ 31, pplayer->base.y) == 'y')
                {
                  add_bouncy_distro((((int)pplayer->base.x + 31)
                                     / 32) * 32,
                                    ((int)pplayer->base.y / 32) * 32);
                  if (counting_distros == NO)
                    {
                      counting_distros = YES;
                      distro_counter = 100;
                    }

                  if (distro_counter <= 0)
                    level_change(&current_level,pplayer->base.x+ 31, pplayer->base.y, 'a');

                  play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                  score = score + SCORE_DISTRO;
                  distros++;
                }
            }
        }

      player_grabdistros(pplayer);
      if(jumped_in_solid == YES)
        {
          ++pplayer->base.y;
          ++pplayer->old_base.y;
          if(player_on_ground(pplayer))
            {
              /* Make sure jumping is off. */
              pplayer->jumping = NO;
            }
        }

    }

  timer_check(&pplayer->safe_timer);


  /* ---- DONE HANDLING TUX! --- */

  /* Handle invincibility timer: */


  if (get_current_music() == HERRING_MUSIC && !timer_check(&pplayer->invincible_timer))
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

  timer_check(&pplayer->skidding_timer);

  /* End of level? */

  if (pplayer->base.x >= endpos && endpos != 0)
    {
      next_level = 1;
    }

}

int player_on_ground(player_type *pplayer)
{
  if( issolid(pplayer->base.x + pplayer->base.width / 2, pplayer->base.y + pplayer->base.height) ||
      issolid(pplayer->base.x + 1, pplayer->base.y + pplayer->base.height) ||
      issolid(pplayer->base.x + pplayer->base.width - 1, pplayer->base.y + pplayer->base.height)  )
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

int player_under_solid(player_type *pplayer)
{
  if( issolid(pplayer->base.x + pplayer->base.width / 2, pplayer->base.y) ||
      issolid(pplayer->base.x + 1, pplayer->base.y) ||
      issolid(pplayer->base.x + pplayer->base.width - 1, pplayer->base.y)  )
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

void player_handle_horizontal_input(player_type *pplayer, int dir)
{

  if ((dir ? (pplayer->base.xm < -SKID_XM) : (pplayer->base.xm > SKID_XM)) && !timer_started(&pplayer->skidding_timer) &&
      pplayer->dir == !dir && player_on_ground(pplayer))
    {
      timer_start(&pplayer->skidding_timer, SKID_TIME);

      play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);

    }
  pplayer->dir = dir;


  if ((dir ? (pplayer->base.xm < 0) : (pplayer->base.xm > 0)) && !isice(pplayer->base.x, pplayer->base.y + pplayer->base.height) &&
      !timer_started(&pplayer->skidding_timer))
    {
      pplayer->base.xm = 0;
    }

  if (!pplayer->duck)
    {
      if (pplayer->dir == dir)
        {
          /* Facing the direction we're jumping?  Go full-speed: */

          if (pplayer->input.fire == UP)
            {
              pplayer->base.xm = pplayer->base.xm + ( dir ? WALK_SPEED : -WALK_SPEED) * frame_ratio;

              if(dir)
                {
                  if (pplayer->base.xm > MAX_WALK_XM)
                    pplayer->base.xm = MAX_WALK_XM;
                }
              else
                {
                  if (pplayer->base.xm < -MAX_WALK_XM)
                    pplayer->base.xm = -MAX_WALK_XM;
                }
            }
          else if ( pplayer->input.fire == DOWN)
            {
              pplayer->base.xm = pplayer->base.xm + ( dir ? RUN_SPEED : -RUN_SPEED) * frame_ratio;

              if(dir)
                {
                  if (pplayer->base.xm > MAX_RUN_XM)
                    pplayer->base.xm = MAX_RUN_XM;
                }
              else
                {
                  if (pplayer->base.xm < -MAX_RUN_XM)
                    pplayer->base.xm = -MAX_RUN_XM;
                }
            }
          else
            {
              /* Not facing the direction we're jumping?
              Go half-speed: */

              pplayer->base.xm = pplayer->base.xm + ( dir ? (WALK_SPEED / 2) : -(WALK_SPEED / 2)) * frame_ratio;

              if(dir)
                {
                  if (pplayer->base.xm > MAX_WALK_XM / 2)
                    pplayer->base.xm = MAX_WALK_XM / 2;
                }
              else
                {
                  if (pplayer->base.xm < -MAX_WALK_XM / 2)
                    pplayer->base.xm = -MAX_WALK_XM / 2;
                }
            }
        }

    }
}

void player_handle_vertical_input(player_type *pplayer)
{
  if(pplayer->input.up == DOWN)
    {
      if (player_on_ground(pplayer))
        {
          if(!physic_is_set(&pplayer->vphysic))
            {
              physic_set_state(&pplayer->vphysic,PH_VT);
              physic_set_start_vy(&pplayer->vphysic,5.5);
              --pplayer->base.y;
              pplayer->jumping = YES;
              if (pplayer->size == SMALL)
                play_sound(sounds[SND_JUMP], SOUND_CENTER_SPEAKER);
              else
                play_sound(sounds[SND_BIGJUMP], SOUND_CENTER_SPEAKER);
            }
        }
    }
  else if(pplayer->input.up == UP && pplayer->jumping == YES)
    {
      if (player_on_ground(pplayer))
        {
          physic_init(&pplayer->vphysic);
          pplayer->jumping == NO;
        }
      else
        {
          pplayer->jumping = NO;
          if(physic_is_set(&pplayer->vphysic))
            {
              if(physic_get_velocity(&pplayer->vphysic) < 0.)
                {
                  physic_set_state(&pplayer->vphysic,PH_VT);
                  physic_set_start_vy(&pplayer->vphysic,0);
                }
            }
          else
            {
              if(!physic_is_set(&pplayer->vphysic))
                {
                  physic_set_state(&pplayer->vphysic,PH_VT);
                }
            }
        }
    }
}

void player_input(player_type *pplayer)
{
  /* Handle key and joystick state: */

  if(pplayer->duck == NO)
    {
      if (pplayer->input.right == DOWN && pplayer->input.left == UP)
        {
          player_handle_horizontal_input(pplayer,RIGHT);
        }
      else if (pplayer->input.left == DOWN && pplayer->input.right == UP)
        {
          player_handle_horizontal_input(pplayer,LEFT);
        }
      else
        {
          if(pplayer->base.xm > 0)
            {
              pplayer->base.xm = (int)(pplayer->base.xm - frame_ratio);
              if(pplayer->base.xm < 0)
                pplayer->base.xm = 0;
            }
          else if(pplayer->base.xm < 0)
            {
              pplayer->base.xm = (int)(pplayer->base.xm + frame_ratio);
              if(pplayer->base.xm > 0)
                pplayer->base.xm = 0;
            }
        }
    }

  /* Jump/jumping? */

  if ( pplayer->input.up == DOWN || (pplayer->input.up == UP && pplayer->jumping == YES))
    {
      player_handle_vertical_input(pplayer);
    }

  /* Shoot! */

  if (pplayer->input.fire == DOWN && pplayer->input.old_fire == UP && pplayer->got_coffee)
    {
      add_bullet(pplayer->base.x, pplayer->base.y, pplayer->base.xm, pplayer->dir);
    }


  /* Duck! */

  if (pplayer->input.down == DOWN)
    {
      if (pplayer->size == BIG && pplayer->duck != YES)
        {
          pplayer->duck = YES;
          pplayer->base.height = 32;
          pplayer->base.y += 32;
        }
    }
  else
    {
      if (pplayer->size == BIG && pplayer->duck == YES)
        {
          /* Make sure we're not standing back up into a solid! */
          pplayer->base.height = 64;
          pplayer->base.y -= 32;

          if (!collision_object_map(&pplayer->base) /*issolid(pplayer->base.x + 16, pplayer->base.y - 16)*/)
            {
              pplayer->duck = NO;
              pplayer->base.height = 64;
              pplayer->old_base.y -= 32;
              pplayer->old_base.height = 64;
            }
          else
            {
              pplayer->base.height = 32;
              pplayer->base.y += 32;
            }
        }
      else
        {
          pplayer->duck = NO;
        }
    }

  /* (Tux): */

  if(!timer_check(&pplayer->frame_timer))
    {
      timer_start(&pplayer->frame_timer,25);
      if (pplayer->input.right == UP && pplayer->input.left == UP)
        {
          pplayer->frame_main = 1;
          pplayer->frame = 1;
        }
      else
        {
          if ((pplayer->input.fire == DOWN && (frame % 2) == 0) ||
              (frame % 4) == 0)
            pplayer->frame_main = (pplayer->frame_main + 1) % 4;

          pplayer->frame = pplayer->frame_main;

          if (pplayer->frame == 3)
            pplayer->frame = 1;
        }
    }

}

void player_grabdistros(player_type *pplayer)
{
  /* Grab distros: */
  if (!pplayer->dying)
    {
      trygrabdistro(pplayer->base.x, pplayer->base.y, NO_BOUNCE);
      trygrabdistro(pplayer->base.x+ 31, pplayer->base.y, NO_BOUNCE);

      trygrabdistro(pplayer->base.x, pplayer->base.y + pplayer->base.height, NO_BOUNCE);
      trygrabdistro(pplayer->base.x+ 31, pplayer->base.y + pplayer->base.height, NO_BOUNCE);

      if(pplayer->size == BIG)
        {
          trygrabdistro(pplayer->base.x, pplayer->base.y + pplayer->base.height / 2, NO_BOUNCE);
          trygrabdistro(pplayer->base.x+ 31, pplayer->base.y + pplayer->base.height / 2, NO_BOUNCE);
        }

    }


  /* Enough distros for a One-up? */

  if (distros >= DISTROS_LIFEUP)
    {
      distros = distros - DISTROS_LIFEUP;
      if(pplayer->lives < MAX_LIVES)
        pplayer->lives++;
      /*We want to hear the sound even, if MAX_LIVES is reached*/
      play_sound(sounds[SND_LIFEUP], SOUND_CENTER_SPEAKER);
    }
}

void player_draw(player_type* pplayer)
{
  if (!timer_started(&pplayer->safe_timer) || (frame % 2) == 0)
    {
      if (pplayer->size == SMALL)
        {
          if (timer_started(&pplayer->invincible_timer))
            {
              /* Draw cape: */

              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&cape_right[frame % 2],
                               pplayer->base.x- scroll_x, pplayer->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&cape_left[frame % 2],
                               pplayer->base.x- scroll_x, pplayer->base.y,
                               NO_UPDATE);
                }
            }


          if (!pplayer->got_coffee)
            {
              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&tux_right[pplayer->frame], pplayer->base.x- scroll_x, pplayer->base.y, NO_UPDATE);
                }
              else
                {
                  texture_draw(&tux_left[pplayer->frame], pplayer->base.x- scroll_x, pplayer->base.y, NO_UPDATE);
                }
            }
          else
            {
              /* Tux got coffee! */

              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&firetux_right[pplayer->frame], pplayer->base.x- scroll_x, pplayer->base.y, NO_UPDATE);
                }
              else
                {
                  texture_draw(&firetux_left[pplayer->frame], pplayer->base.x- scroll_x, pplayer->base.y, NO_UPDATE);
                }
            }
        }
      else
        {
          if (timer_started(&pplayer->invincible_timer))
            {
              /* Draw cape: */

              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&bigcape_right[frame % 2],
                               pplayer->base.x- scroll_x - 8, pplayer->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&bigcape_left[frame % 2],
                               pplayer->base.x-scroll_x - 8, pplayer->base.y,
                               NO_UPDATE);
                }
            }

          if (!pplayer->got_coffee)
            {
              if (!pplayer->duck)
                {
                  if (!timer_started(&pplayer->skidding_timer))
                    {
                      if (!pplayer->jumping || pplayer->base.ym > 0)
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigtux_right[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigtux_left[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                        }
                      else
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigtux_right_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigtux_left_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                        }
                    }
                  else
                    {
                      if (pplayer->dir == RIGHT)
                        {
                          texture_draw(&skidtux_right,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                       NO_UPDATE);
                        }
                      else
                        {
                          texture_draw(&skidtux_left,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                       NO_UPDATE);
                        }
                    }
                }
              else
                {
                  if (pplayer->dir == RIGHT)
                    {
                      texture_draw(&ducktux_right, pplayer->base.x- scroll_x - 8, pplayer->base.y - 16,
                                   NO_UPDATE);
                    }
                  else
                    {
                      texture_draw(&ducktux_left, pplayer->base.x- scroll_x - 8, pplayer->base.y - 16,
                                   NO_UPDATE);
                    }
                }
            }
          else
            {
              /* Tux has coffee! */

              if (!pplayer->duck)
                {
                  if (!timer_started(&pplayer->skidding_timer))
                    {
                      if (!pplayer->jumping || pplayer->base.ym > 0)
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                        }
                      else
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                           NO_UPDATE);
                            }
                        }
                    }
                  else
                    {
                      if (pplayer->dir == RIGHT)
                        {
                          texture_draw(&skidfiretux_right,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                       NO_UPDATE);
                        }
                      else
                        {
                          texture_draw(&skidfiretux_left,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y,
                                       NO_UPDATE);
                        }
                    }
                }
              else
                {
                  if (pplayer->dir == RIGHT)
                    {
                      texture_draw(&duckfiretux_right, pplayer->base.x- scroll_x - 8, pplayer->base.y - 16,
                                   NO_UPDATE);
                    }
                  else
                    {
                      texture_draw(&duckfiretux_left, pplayer->base.x- scroll_x - 8, pplayer->base.y - 16,
                                   NO_UPDATE);
                    }
                }
            }
        }
    }
}

void player_collision(player_type* pplayer, void* p_c_object, int c_object)
{
  bad_guy_type* pbad_c = NULL;

  switch (c_object)
    {
    case CO_BADGUY:
      pbad_c = (bad_guy_type*) p_c_object;
      /* Hurt the player if he just touched it: */

      if (!pbad_c->dying && !pplayer->dying &&
          !timer_started(&pplayer->safe_timer) &&
          pbad_c->mode != HELD)
        {
          if (pbad_c->mode == FLAT  && pplayer->input.fire != DOWN)
            {
              /* Kick: */

              pbad_c->mode = KICK;
              play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

              if (pplayer->base.x<= pbad_c->base.x)
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
          else if (pbad_c->mode == FLAT && pplayer->input.fire == DOWN)
            {
              pbad_c->mode = HELD;
              pbad_c->base.y-=8;
            }
          else if (pbad_c->mode == KICK)
            {
              if (pplayer->base.y < pbad_c->base.y - 16 &&
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
                      if (!timer_started(&pplayer->invincible_timer))
                        {
                          player_kill(pplayer,SHRINK);
                        }
                      else
                        {
                          pbad_c->dying = FALLING;
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
              if (!timer_started(&pplayer->invincible_timer ))
                {
                  player_kill(pplayer,SHRINK);
                }
              else
                {
                  pbad_c->dying = FALLING;
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

void player_kill(player_type* pplayer, int mode)
{
  pplayer->base.ym = -5;

  play_sound(sounds[SND_HURT], SOUND_CENTER_SPEAKER);

  if (pplayer->dir == RIGHT)
    pplayer->base.xm = -8;
  else if (pplayer->dir == LEFT)
    pplayer->base.xm = 8;

  if (mode == SHRINK && pplayer->size == BIG)
    {
      if (pplayer->got_coffee)
        pplayer->got_coffee = NO;

      pplayer->size = SMALL;
      pplayer->base.height = 32;

      timer_start(&pplayer->safe_timer,TUX_SAFE_TIME);
    }
  else
    {
      pplayer->dying = 1;
    }
}

void player_dying(player_type *pplayer)
{
  pplayer->base.ym = pplayer->base.ym + gravity;

  /* He died :^( */

  --pplayer->lives;
  player_remove_powerups(pplayer);
  pplayer->dying = NO;

  player_level_begin(pplayer);

}

/* Remove Tux's power ups */
void player_remove_powerups(player_type* pplayer)
{
  pplayer->got_coffee = NO;
  pplayer->size = SMALL;
  pplayer->base.height = 32;
}

void player_keep_in_bounds(player_type* pplayer)
{
  /* Keep tux in bounds: */
  if (pplayer->base.x< 0)
    pplayer->base.x= 0;
  else if(pplayer->base.x< scroll_x)
    pplayer->base.x= scroll_x;
  else if (pplayer->base.x< 160 + scroll_x && scroll_x > 0 && debug_mode == YES)
    {
      scroll_x = pplayer->base.x- 160;
      /*pplayer->base.x+= 160;*/

      if(scroll_x < 0)
        scroll_x = 0;

    }
  else if (pplayer->base.x> screen->w / 2 + scroll_x && scroll_x < ((current_level.width * 32) - screen->w))
    {
      /* Scroll the screen in past center: */

      scroll_x = pplayer->base.x- screen->w / 2;
      /*pplayer->base.x= 320 + scroll_x;*/

      if (scroll_x > ((current_level.width * 32) - screen->w))
        scroll_x = ((current_level.width * 32) - screen->w);
    }
  else if (pplayer->base.x> 608 + scroll_x)
    {
      /* ... unless there's no more to scroll! */

      /*pplayer->base.x= 608 + scroll_x;*/
    }

  /* Keep in-bounds, vertically: */

  if (pplayer->base.y > screen->h)
    {
      player_kill(&tux,KILL);
    }
}
