//
// C Implementation: player/tux
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
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

void player_init(player_type* pplayer)
{
  pplayer->it.alive = NULL;
  pplayer->it.x = &pplayer->x;
  pplayer->it.y = &pplayer->y;
  pplayer->it.width = &pplayer->width;
  pplayer->it.height = &pplayer->height;
  pplayer->it.updated = &pplayer->updated;

  pplayer->width = 32;
  pplayer->height = 32;

  pplayer->updated = SDL_GetTicks();
  pplayer->size = SMALL;
  pplayer->got_coffee = NO;

  pplayer->x = 0;
  pplayer->xm = 0;
  pplayer->y = 240;
  pplayer->ym = 0;
  pplayer->dir = RIGHT;
  pplayer->duck = NO;

  pplayer->dying = NO;
  pplayer->safe = TUX_SAFE_TIME;

  pplayer->jumping = NO;
  pplayer->skidding = 0;

  pplayer->frame_main = 0;
  pplayer->frame = 0;
  pplayer->lives = 3;

  pplayer->input.down = UP;
  pplayer->input.fire = UP;
  pplayer->input.left = UP;
  pplayer->input.old_fire = UP;
  pplayer->input.right = UP;
  pplayer->input.up = UP;

  timer_init(&pplayer->invincible_timer);
}

void player_level_begin(player_type* pplayer)
{
  pplayer->x = 0;
  pplayer->xm = 0;
  pplayer->y = 240;
  pplayer->ym = 0;
}

void player_action(player_type* pplayer)
{

  double frame_ratio = get_frame_ratio(&pplayer->it);

  /* --- HANDLE TUX! --- */

  player_input(pplayer);

  /* Move tux: */

  pplayer->x = pplayer->x + pplayer->xm * frame_ratio;
  pplayer->y = pplayer->y + pplayer->ym * frame_ratio;

  player_keep_in_bounds(pplayer);

  /* Land: */

  if (!pplayer->dying)
    {
      if (issolid(pplayer->x, pplayer->y + 31) &&
          !issolid(pplayer->x - pplayer->xm, pplayer->y + 31))
        {
          while (issolid(pplayer->x, pplayer->y + 31))
            {
              if (pplayer->xm < 0)
                pplayer->x++;
              else if (pplayer->xm > 0)
                pplayer->x--;
            }

          pplayer->xm = 0;
        }

      if (issolid(pplayer->x, pplayer->y) &&
          !issolid(pplayer->x - pplayer->xm, pplayer->y))
        {
          while (issolid(pplayer->x, (pplayer->y)))
            {
              if (pplayer->xm < 0)
                pplayer->x++;
              else if (pplayer->xm > 0)
                pplayer->x--;
            }

          pplayer->xm = 0;
        }

      if (issolid(pplayer->x, pplayer->y + 31))
        {
          /* Set down properly: */

          int debug_int = 0;
          while (issolid(pplayer->x, pplayer->y + 31))
            {
              ++debug_int;
              if(debug_int > 32)
                DEBUG_MSG("FIXME - UNDER certain circumstances I'm hanging in a loop here!");
		
              if (pplayer->ym < 0)
                pplayer->y++;
              else if (pplayer->ym > 0)
                pplayer->y--;
            }


          /* Reset score multiplier (for multi-hits): */

          if (pplayer->ym > 0)
            score_multiplier = 1;


          /* Stop jumping! */

          pplayer->ym = 0;
          pplayer->jumping = NO;
        }


      /* Bump into things: */

      if (issolid(pplayer->x, pplayer->y) ||
          (pplayer->size == BIG && !pplayer->duck &&
           (issolid(pplayer->x, pplayer->y - 32))))
        {
          if (!issolid(pplayer->x - pplayer->xm, pplayer->y) &&
              (pplayer->size == SMALL || pplayer->duck ||
               !issolid(pplayer->x - pplayer->xm, pplayer->y - 32)))
            {
              pplayer->x = pplayer->x - pplayer->xm;
              pplayer->xm = 0;
            }
          else if (!issolid(pplayer->x, pplayer->y - pplayer->ym) &&
                   (pplayer->size == SMALL || pplayer->duck ||
                    !issolid(pplayer->x, pplayer->y - 32 - pplayer->ym)))
            {
              if (pplayer->ym <= 0)
                {
                  /* Jumping up? */

                  if (pplayer->size == BIG)
                    {
                      /* Break bricks and empty boxes: */

                      if (!pplayer->duck)
                        {
                          if (isbrick(pplayer->x, pplayer->y - 32) ||
                              isfullbox(pplayer->x, pplayer->y - 32))
                            {
                              trygrabdistro(pplayer->x, pplayer->y - 64, BOUNCE);
                              trybumpbadguy(pplayer->x, pplayer->y - 96);

                              if (isfullbox(pplayer->x, pplayer->y - 32))
                                {
                                  bumpbrick(pplayer->x, pplayer->y - 32);
                                }

                              trybreakbrick(pplayer->x, pplayer->y - 32);
                              tryemptybox(pplayer->x, pplayer->y - 32);
                            }

                          if (isbrick(pplayer->x + 31, pplayer->y - 32) ||
                              isfullbox(pplayer->x + 31, pplayer->y - 32))
                            {
                              trygrabdistro(pplayer->x + 31,
                                            pplayer->y - 64,
                                            BOUNCE);
                              trybumpbadguy(pplayer->x + 31,
                                            pplayer->y - 96);

                              if (isfullbox(pplayer->x + 31, pplayer->y - 32))
                                {
                                  bumpbrick(pplayer->x + 31, pplayer->y - 32);
                                }

                              trybreakbrick(pplayer->x + 31,
                                            pplayer->y - 32);
                              tryemptybox(pplayer->x + 31,
                                          pplayer->y - 32);
                            }
                        }
                      else /* ducking */
                        {
                          if (isbrick(pplayer->x, pplayer->y) ||
                              isfullbox(pplayer->x, pplayer->y))
                            {
                              trygrabdistro(pplayer->x, pplayer->y - 32,BOUNCE);
                              trybumpbadguy(pplayer->x, pplayer->y - 64);
                              if (isfullbox(pplayer->x, pplayer->y))
                                bumpbrick(pplayer->x, pplayer->y);
                              trybreakbrick(pplayer->x, pplayer->y);
                              tryemptybox(pplayer->x, pplayer->y);
                            }

                          if (isbrick(pplayer->x + 31, pplayer->y) ||
                              isfullbox(pplayer->x + 31, pplayer->y))
                            {
                              trygrabdistro(pplayer->x + 31,
                                            pplayer->y - 32,
                                            BOUNCE);
                              trybumpbadguy(pplayer->x + 31,
                                            pplayer->y - 64);
                              if (isfullbox(pplayer->x + 31, pplayer->y))
                                bumpbrick(pplayer->x + 31, pplayer->y);
                              trybreakbrick(pplayer->x + 31, pplayer->y);
                              tryemptybox(pplayer->x + 31, pplayer->y);
                            }
                        }
                    }
                  else
                    {
                      /* It's a brick and we're small, make the brick
                         bounce, and grab any distros above it: */

                      if (isbrick(pplayer->x, pplayer->y) ||
                          isfullbox(pplayer->x, pplayer->y))
                        {
                          trygrabdistro(pplayer->x, pplayer->y - 32,BOUNCE);
                          trybumpbadguy(pplayer->x, pplayer->y - 64);
                          bumpbrick(pplayer->x, pplayer->y);
                          tryemptybox(pplayer->x, pplayer->y);
                        }

                      if (isbrick(pplayer->x + 31, pplayer->y) ||
                          isfullbox(pplayer->x + 31, pplayer->y))
                        {
                          trygrabdistro(pplayer->x + 31, pplayer->y - 32,BOUNCE);
                          trybumpbadguy(pplayer->x + 31, pplayer->y - 64);
                          bumpbrick(pplayer->x + 31, pplayer->y);
                          tryemptybox(pplayer->x + 31, pplayer->y);
                        }


                      /* Get a distro from a brick? */

                      if (shape(pplayer->x, pplayer->y) == 'x' ||
                          shape(pplayer->x, pplayer->y) == 'y')
                        {
                          add_bouncy_distro(((pplayer->x + 1)
                                             / 32) * 32,
                                            (int)(pplayer->y / 32) * 32);

                          if (counting_distros == NO)
                            {
                              counting_distros = YES;
                              distro_counter = 100;
                            }

                          if (distro_counter <= 0)
                            level_change(&current_level,pplayer->x, pplayer->y, 'a');

                          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                          score = score + SCORE_DISTRO;
                          distros++;
                        }
                      else if (shape(pplayer->x + 31, pplayer->y) == 'x' ||
                               shape(pplayer->x + 31, pplayer->y) == 'y')
                        {
                          add_bouncy_distro(((pplayer->x + 1 + 31)
                                             / 32) * 32,
                                            (int)(pplayer->y / 32) * 32);

                          if (counting_distros == NO)
                            {
                              counting_distros = YES;
                              distro_counter = 100;
                            }

                          if (distro_counter <= 0)
                            level_change(&current_level,pplayer->x + 31 + scroll_x, pplayer->y, 'a');

                          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                          score = score + SCORE_DISTRO;
                          distros++;
                        }
                    }


                  /* Bump head: */

                  pplayer->y = (int)(pplayer->y / 32) * 32 + 30;
                }
              else
                {
                  /* Land on feet: */

                  pplayer->y = (int)(pplayer->y / 32) * 32 - 32;
                }

              pplayer->ym = 0;
              pplayer->jumping = NO;
              /*pplayer->jump_counter = MAX_JUMP_COUNT;*/
              /*timer_init(&pplayer->jump_timer);*/
              timer_start(&pplayer->jump_timer,MAX_JUMP_TIME);
            }
        }
    }



  player_grabdistros(pplayer);


  /* Slow down horizontally: */

  if (!pplayer->dying)
    {
      if (pplayer->input.right == UP && pplayer->input.left == UP)
        {
          if (isice(pplayer->x, pplayer->y + 32) ||
              !issolid(pplayer->x, pplayer->y + 32))
            {
              /* Slowly on ice or in air: */

              if (pplayer->xm > 0)
                pplayer->xm--;
              else if (pplayer->xm < 0)
                pplayer->xm++;
            }
          else
            {
              /* Quickly, otherwise: */

              pplayer->xm = pplayer->xm / 2;
            }
        }


      /* Drop vertically: */

      if (!issolid(pplayer->x, pplayer->y + 32))
        {
          pplayer->ym = pplayer->ym + GRAVITY;

          if (pplayer->ym > MAX_YM)
            pplayer->ym = MAX_YM;
        }
    }



  if (pplayer->safe > 0)
    pplayer->safe--;

  /* ---- DONE HANDLING TUX! --- */

  /* Handle invincibility timer: */


  if (timer_check(&pplayer->invincible_timer))
    {
      if (current_music == HERRING_MUSIC)
        {
          if (current_level.time_left <= TIME_WARNING)
            {
              /* stop the herring_song, prepare to play the correct
               * fast level_song !
               */
              current_music = HURRYUP_MUSIC;
            }
          else
            {
              current_music = LEVEL_MUSIC;
            }
          /* stop the old music if it's being played */
          if (playing_music())
            halt_music();
        }
    }

  /* Handle skidding: */

  if (pplayer->skidding > 0)
    {
      pplayer->skidding--;
    }

  /* End of level? */

  if (pplayer->x >= endpos && endpos != 0)
    {
      next_level = 1;
    }

}

void player_input(player_type *pplayer)
{
  /* Handle key and joystick state: */


  if (pplayer->input.right == DOWN && pplayer->input.left == UP)
    {
      if (pplayer->jumping == NO)
        {
          if (pplayer->xm < -SKID_XM && !pplayer->skidding &&
              pplayer->dir == LEFT)
            {
              pplayer->skidding = SKID_TIME;

              play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);

            }
          pplayer->dir = RIGHT;
        }

      if (pplayer->xm < 0 && !isice(pplayer->x, pplayer->y + 32) &&
          !pplayer->skidding)
        {
          pplayer->xm = 0;
        }

      if (!pplayer->duck)
        {
          if (pplayer->dir == RIGHT)
            {
              /* Facing the direction we're jumping?  Go full-speed: */

              if (pplayer->input.fire == UP)
                {
                  pplayer->xm = pplayer->xm + WALK_SPEED;

                  if (pplayer->xm > MAX_WALK_XM)
                    pplayer->xm = MAX_WALK_XM;
                }
              else if ( pplayer->input.fire == DOWN)
                {
                  pplayer->xm = pplayer->xm + RUN_SPEED;

                  if (pplayer->xm > MAX_RUN_XM)
                    pplayer->xm = MAX_RUN_XM;
                }
            }
          else
            {
              /* Not facing the direction we're jumping?
              Go half-speed: */

              pplayer->xm = pplayer->xm + WALK_SPEED / 2;

              if (pplayer->xm > MAX_WALK_XM / 2)
                pplayer->xm = MAX_WALK_XM / 2;
            }
        }
    }
  else if (pplayer->input.left == DOWN && pplayer->input.right == UP)
    {
      if (pplayer->jumping == NO)
        {
          if (pplayer->xm > SKID_XM && !pplayer->skidding &&
              pplayer->dir == RIGHT)
            {
              pplayer->skidding = SKID_TIME;
              play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);
            }
          pplayer->dir = LEFT;
        }

      if (pplayer->xm > 0 && !isice(pplayer->x, pplayer->y + 32) &&
          !pplayer->skidding)
        {
          pplayer->xm = 0;
        }

      if (!pplayer->duck)
        {
          if (pplayer->dir == LEFT)
            {
              /* Facing the direction we're jumping?  Go full-speed: */

              if (pplayer->input.fire == UP)
                {
                  pplayer->xm = pplayer->xm - WALK_SPEED;

                  if (pplayer->xm < -MAX_WALK_XM)
                    pplayer->xm = -MAX_WALK_XM;
                }
              else if (pplayer->input.fire == DOWN)
                {
                  pplayer->xm = pplayer->xm - RUN_SPEED;

                  if (pplayer->xm < -MAX_RUN_XM)
                    pplayer->xm = -MAX_RUN_XM;
                }
            }
          else
            {
              /* Not facing the direction we're jumping?
              Go half-speed: */

              pplayer->xm = pplayer->xm - WALK_SPEED / 2;

              if (pplayer->xm < -MAX_WALK_XM / 2)
                pplayer->xm = -MAX_WALK_XM / 2;
            }
        }
    }

  /* Jump/jumping? */

  if ( pplayer->input.up == DOWN)
    {
      if(!timer_started(&pplayer->jump_timer))
        {
          timer_start(&pplayer->jump_timer,MAX_JUMP_TIME);


          /* Taking off? */

          if (!issolid(pplayer->x, pplayer->y + 32) ||
              pplayer->ym != 0)
            {
              /* If they're not on the ground, or are currently moving
              vertically, don't jump! */

              pplayer->jumping = NO;
              timer_stop(&pplayer->jump_timer);
            }
          else
            {
              /* Make sure we're not standing back up into a solid! */

              if (pplayer->size == SMALL || pplayer->duck == NO ||
                  !issolid(pplayer->x, pplayer->y))
                {
                  pplayer->jumping = YES;

                  if (pplayer->size == SMALL)
                    play_sound(sounds[SND_JUMP], SOUND_CENTER_SPEAKER);
                  else
                    play_sound(sounds[SND_BIGJUMP], SOUND_CENTER_SPEAKER);
                }
            }
        }

      /* Keep jumping for a while: */

      if (timer_check(&pplayer->jump_timer))
        {
          pplayer->ym = pplayer->ym - JUMP_SPEED;
        }
    }
  else
    timer_stop(&pplayer->jump_timer);


  /* Shoot! */

  if (pplayer->input.fire == DOWN && pplayer->input.old_fire == UP && pplayer->got_coffee)
    {
      add_bullet(pplayer->x, pplayer->y, pplayer->xm, pplayer->dir);
    }


  /* Duck! */

  if (pplayer->input.down == DOWN)
    {
      if (pplayer->size == BIG)
        pplayer->duck = YES;
    }
  else
    {
      if (pplayer->size == BIG && pplayer->duck == YES)
        {
          /* Make sure we're not standing back up into a solid! */

          if (!issolid(pplayer->x, pplayer->y - 32))
            pplayer->duck = NO;
        }
      else
        pplayer->duck = NO;
    }

  /* (Tux): */

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

void player_grabdistros(player_type *pplayer)
{
  /* Grab distros: */
  if (!pplayer->dying)
    {
      trygrabdistro(pplayer->x , pplayer->y, NO_BOUNCE);
      trygrabdistro(pplayer->x + 31, pplayer->y, NO_BOUNCE);

      if (pplayer->size == BIG && !pplayer->duck)
        {
          trygrabdistro(pplayer->x, pplayer->y - 32, NO_BOUNCE);
          trygrabdistro(pplayer->x + 31, pplayer->y - 32, NO_BOUNCE);
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

  if (pplayer->safe == 0 || (frame % 2) == 0)
    {
      if (pplayer->size == SMALL)
        {
          if (timer_started(&pplayer->invincible_timer))
            {
              /* Draw cape: */

              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&cape_right[frame % 2],
                               pplayer->x - scroll_x, pplayer->y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&cape_left[frame % 2],
                               pplayer->x - scroll_x, pplayer->y,
                               NO_UPDATE);
                }
            }


          if (!pplayer->got_coffee)
            {
              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&tux_right[pplayer->frame], pplayer->x - scroll_x, pplayer->y, NO_UPDATE);
                }
              else
                {
                  texture_draw(&tux_left[pplayer->frame], pplayer->x - scroll_x, pplayer->y, NO_UPDATE);
                }
            }
          else
            {
              /* Tux got coffee! */

              if (pplayer->dir == RIGHT)
                {
                  texture_draw(&firetux_right[pplayer->frame], pplayer->x - scroll_x, pplayer->y, NO_UPDATE);
                }
              else
                {
                  texture_draw(&firetux_left[pplayer->frame], pplayer->x - scroll_x, pplayer->y, NO_UPDATE);
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
                               pplayer->x - scroll_x - 8 - 16, pplayer->y - 32,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&bigcape_left[frame % 2],
                               pplayer->x -scroll_x - 8, pplayer->y - 32,
                               NO_UPDATE);
                }
            }

          if (!pplayer->got_coffee)
            {
              if (!pplayer->duck)
                {
                  if (!pplayer->skidding)
                    {
                      if (!pplayer->jumping || pplayer->ym > 0)
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigtux_right[pplayer->frame],
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigtux_left[pplayer->frame],
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                        }
                      else
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigtux_right_jump,
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigtux_left_jump,
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                        }
                    }
                  else
                    {
                      if (pplayer->dir == RIGHT)
                        {
                          texture_draw(&skidtux_right,
                                       pplayer->x - scroll_x - 8, pplayer->y - 32,
                                       NO_UPDATE);
                        }
                      else
                        {
                          texture_draw(&skidtux_left,
                                       pplayer->x - scroll_x - 8, pplayer->y - 32,
                                       NO_UPDATE);
                        }
                    }
                }
              else
                {
                  if (pplayer->dir == RIGHT)
                    {
                      texture_draw(&ducktux_right, pplayer->x - scroll_x - 8, pplayer->y - 16,
                                   NO_UPDATE);
                    }
                  else
                    {
                      texture_draw(&ducktux_left, pplayer->x - scroll_x - 8, pplayer->y - 16,
                                   NO_UPDATE);
                    }
                }
            }
          else
            {
              /* Tux has coffee! */

              if (!pplayer->duck)
                {
                  if (!pplayer->skidding)
                    {
                      if (!pplayer->jumping || pplayer->ym > 0)
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right[pplayer->frame],
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left[pplayer->frame],
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                        }
                      else
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right_jump,
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left_jump,
                                           pplayer->x - scroll_x - 8, pplayer->y - 32,
                                           NO_UPDATE);
                            }
                        }
                    }
                  else
                    {
                      if (pplayer->dir == RIGHT)
                        {
                          texture_draw(&skidfiretux_right,
                                       pplayer->x - scroll_x - 8, pplayer->y - 32,
                                       NO_UPDATE);
                        }
                      else
                        {
                          texture_draw(&skidfiretux_left,
                                       pplayer->x - scroll_x - 8, pplayer->y - 32,
                                       NO_UPDATE);
                        }
                    }
                }
              else
                {
                  if (pplayer->dir == RIGHT)
                    {
                      texture_draw(&duckfiretux_right, pplayer->x - scroll_x - 8, pplayer->y - 16,
                                   NO_UPDATE);
                    }
                  else
                    {
                      texture_draw(&duckfiretux_left, pplayer->x - scroll_x - 8, pplayer->y - 16,
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
      pbad_c = p_c_object;
      /* Hurt the player if he just touched it: */

      if (!pbad_c->dying && !pplayer->dying &&
          !pplayer->safe &&
          pbad_c->mode != HELD)
        {
          if (pbad_c->mode == FLAT  && pplayer->input.fire != DOWN)
            {
              /* Kick: */

              pbad_c->mode = KICK;
              play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

              if (pplayer->x <= pbad_c->x)
                {
                  pbad_c->dir = RIGHT;
                  pbad_c->x = pbad_c->x + 16;
                }
              else
                {
                  pbad_c->dir = LEFT;
                  pbad_c->x = pbad_c->x - 16;
                }

              timer_start(&pbad_c->timer,5000);
            }
          else if (pbad_c->mode == FLAT && pplayer->input.fire == DOWN)
            {
              pbad_c->mode = HELD;
              pbad_c->y-=8;
            }
          else if (pbad_c->mode == KICK)
            {
              if (pplayer->y < pbad_c->y - 16 &&
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
                          pbad_c->ym = -8;
                          play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
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
                  pbad_c->ym = -8;
                  play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
                }
            }
        }
      score_multiplier++;
      break;
    }

}

/* Kill Player! */

void player_kill(player_type* pplayer, int mode)
{
  pplayer->ym = -5;

  play_sound(sounds[SND_HURT], SOUND_CENTER_SPEAKER);

  if (pplayer->dir == RIGHT)
    pplayer->xm = -8;
  else if (tux.dir == LEFT)
    pplayer->xm = 8;

  if (mode == SHRINK && pplayer->size == BIG)
    {
      if (pplayer->got_coffee)
        pplayer->got_coffee = NO;

      pplayer->size = SMALL;

      pplayer->safe = TUX_SAFE_TIME;
    }
  else
    {
      pplayer->dying = 1;
    }
}

void player_dying(player_type *pplayer)
{
  pplayer->ym = pplayer->ym + GRAVITY;

  /* He died :^( */

  --pplayer->lives;
  player_remove_powerups(pplayer);
  pplayer->dying = 0;

}

/* Remove Tux's power ups */
void player_remove_powerups(player_type* pplayer)
{
  pplayer->got_coffee = NO;
  pplayer->size = SMALL;
}

void player_keep_in_bounds(player_type* pplayer)
{
  /* Keep tux in bounds: */
  if (pplayer->x < 0)
    pplayer->x = 0;
  else if(pplayer->x < scroll_x)
    pplayer->x = scroll_x;
  else if (pplayer->x < 160 + scroll_x && scroll_x > 0 && debug_mode == YES)
    {
      scroll_x = pplayer->x - 160;
      /*pplayer->x += 160;*/

      if(scroll_x < 0)
        scroll_x = 0;

    }
  else if (pplayer->x > screen->w / 2 + scroll_x && scroll_x < ((current_level.width * 32) - screen->w))
    {
      /* Scroll the screen in past center: */

      scroll_x = pplayer->x - screen->w / 2;
      /*pplayer->x = 320 + scroll_x;*/

      if (scroll_x > ((current_level.width * 32) - screen->w))
        scroll_x = ((current_level.width * 32) - screen->w);
    }
  else if (pplayer->x > 608 + scroll_x)
    {
      /* ... unless there's no more to scroll! */

      /*pplayer->x = 608 + scroll_x;*/
    }

  /* Keep in-bounds, vertically: */

  if (pplayer->y < 0)
    pplayer->y = 0;
  else if (pplayer->y > screen->h)
    {
      player_kill(&tux,KILL);
    }
}
