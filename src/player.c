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
  pplayer->base.width = 32;
  pplayer->base.height = 32;

  pplayer->base.updated = SDL_GetTicks();
  pplayer->size = SMALL;
  pplayer->got_coffee = NO;

  pplayer->base.x = 0;
  pplayer->base.y = 240;
  pplayer->base.xm = 0;
  pplayer->base.ym = 0;
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
  pplayer->base.x = 0;
  pplayer->base.y = 240;
  pplayer->base.xm = 0;
  pplayer->base.ym = 0;
}

void player_action(player_type* pplayer)
{

  double frame_ratio = get_frame_ratio(&pplayer->base);

  /* --- HANDLE TUX! --- */

  player_input(pplayer);

  /* Move tux: */

  pplayer->base.x= pplayer->base.x+ pplayer->base.xm * frame_ratio;
  pplayer->base.y = pplayer->base.y + pplayer->base.ym * frame_ratio;

  player_keep_in_bounds(pplayer);

  /* Land: */

  if (!pplayer->dying)
    {
      if (issolid(pplayer->base.x, pplayer->base.y + 31) &&
          !issolid(pplayer->base.x- pplayer->base.xm, pplayer->base.y + 31))
        {
          while (issolid(pplayer->base.x, pplayer->base.y + 31))
            {
              if (pplayer->base.xm < 0)
                pplayer->base.x++;
              else if (pplayer->base.xm > 0)
                pplayer->base.x--;
            }

          pplayer->base.xm = 0;
        }

      if (issolid(pplayer->base.x, pplayer->base.y) &&
          !issolid(pplayer->base.x- pplayer->base.xm, pplayer->base.y))
        {
          while (issolid(pplayer->base.x, (pplayer->base.y)))
            {
              if (pplayer->base.xm < 0)
                pplayer->base.x++;
              else if (pplayer->base.xm > 0)
                pplayer->base.x--;
            }

          pplayer->base.xm = 0;
        }

      if (issolid(pplayer->base.x, pplayer->base.y + 31))
        {
          /* Set down properly: */

          int debug_int = 0;
          while (issolid(pplayer->base.x, pplayer->base.y + 31))
            {
              ++debug_int;
              if(debug_int > 32)
                DEBUG_MSG("FIXME - UNDER certain circumstances I'm hanging in a loop here!");
		
              if (pplayer->base.ym < 0)
                pplayer->base.y++;
              else if (pplayer->base.ym > 0)
                pplayer->base.y--;
            }


          /* Reset score multiplier (for multi-hits): */

          if (pplayer->base.ym > 0)
            score_multiplier = 1;


          /* Stop jumping! */

          pplayer->base.ym = 0;
          pplayer->jumping = NO;
        }


      /* Bump into things: */

      if (issolid(pplayer->base.x, pplayer->base.y) ||
          (pplayer->size == BIG && !pplayer->duck &&
           (issolid(pplayer->base.x, pplayer->base.y - 32))))
        {
          if (!issolid(pplayer->base.x- pplayer->base.xm, pplayer->base.y) &&
              (pplayer->size == SMALL || pplayer->duck ||
               !issolid(pplayer->base.x- pplayer->base.xm, pplayer->base.y - 32)))
            {
              pplayer->base.x = pplayer->base.x- pplayer->base.xm;
              pplayer->base.xm = 0;
            }
          else if (!issolid(pplayer->base.x, pplayer->base.y - pplayer->base.ym) &&
                   (pplayer->size == SMALL || pplayer->duck ||
                    !issolid(pplayer->base.x, pplayer->base.y - 32 - pplayer->base.ym)))
            {
              if (pplayer->base.ym <= 0)
                {
                  /* Jumping up? */

                  if (pplayer->size == BIG)
                    {
                      /* Break bricks and empty boxes: */

                      if (!pplayer->duck)
                        {
                          if (isbrick(pplayer->base.x, pplayer->base.y - 32) ||
                              isfullbox(pplayer->base.x, pplayer->base.y - 32))
                            {
                              trygrabdistro(pplayer->base.x, pplayer->base.y - 64, BOUNCE);
                              trybumpbadguy(pplayer->base.x, pplayer->base.y - 96);

                              if (isfullbox(pplayer->base.x, pplayer->base.y - 32))
                                {
                                  bumpbrick(pplayer->base.x, pplayer->base.y - 32);
                                }

                              trybreakbrick(pplayer->base.x, pplayer->base.y - 32);
                              tryemptybox(pplayer->base.x, pplayer->base.y - 32);
                            }

                          if (isbrick(pplayer->base.x+ 31, pplayer->base.y - 32) ||
                              isfullbox(pplayer->base.x+ 31, pplayer->base.y - 32))
                            {
                              trygrabdistro(pplayer->base.x+ 31,
                                            pplayer->base.y - 64,
                                            BOUNCE);
                              trybumpbadguy(pplayer->base.x+ 31,
                                            pplayer->base.y - 96);

                              if (isfullbox(pplayer->base.x+ 31, pplayer->base.y - 32))
                                {
                                  bumpbrick(pplayer->base.x+ 31, pplayer->base.y - 32);
                                }

                              trybreakbrick(pplayer->base.x+ 31,
                                            pplayer->base.y - 32);
                              tryemptybox(pplayer->base.x+ 31,
                                          pplayer->base.y - 32);
                            }
                        }
                      else /* ducking */
                        {
                          if (isbrick(pplayer->base.x, pplayer->base.y) ||
                              isfullbox(pplayer->base.x, pplayer->base.y))
                            {
                              trygrabdistro(pplayer->base.x, pplayer->base.y - 32,BOUNCE);
                              trybumpbadguy(pplayer->base.x, pplayer->base.y - 64);
                              if (isfullbox(pplayer->base.x, pplayer->base.y))
                                bumpbrick(pplayer->base.x, pplayer->base.y);
                              trybreakbrick(pplayer->base.x, pplayer->base.y);
                              tryemptybox(pplayer->base.x, pplayer->base.y);
                            }

                          if (isbrick(pplayer->base.x+ 31, pplayer->base.y) ||
                              isfullbox(pplayer->base.x+ 31, pplayer->base.y))
                            {
                              trygrabdistro(pplayer->base.x+ 31,
                                            pplayer->base.y - 32,
                                            BOUNCE);
                              trybumpbadguy(pplayer->base.x+ 31,
                                            pplayer->base.y - 64);
                              if (isfullbox(pplayer->base.x+ 31, pplayer->base.y))
                                bumpbrick(pplayer->base.x+ 31, pplayer->base.y);
                              trybreakbrick(pplayer->base.x+ 31, pplayer->base.y);
                              tryemptybox(pplayer->base.x+ 31, pplayer->base.y);
                            }
                        }
                    }
                  else
                    {
                      /* It's a brick and we're small, make the brick
                         bounce, and grab any distros above it: */

                      if (isbrick(pplayer->base.x, pplayer->base.y) ||
                          isfullbox(pplayer->base.x, pplayer->base.y))
                        {
                          trygrabdistro(pplayer->base.x, pplayer->base.y - 32,BOUNCE);
                          trybumpbadguy(pplayer->base.x, pplayer->base.y - 64);
                          bumpbrick(pplayer->base.x, pplayer->base.y);
                          tryemptybox(pplayer->base.x, pplayer->base.y);
                        }

                      if (isbrick(pplayer->base.x+ 31, pplayer->base.y) ||
                          isfullbox(pplayer->base.x+ 31, pplayer->base.y))
                        {
                          trygrabdistro(pplayer->base.x+ 31, pplayer->base.y - 32,BOUNCE);
                          trybumpbadguy(pplayer->base.x+ 31, pplayer->base.y - 64);
                          bumpbrick(pplayer->base.x+ 31, pplayer->base.y);
                          tryemptybox(pplayer->base.x+ 31, pplayer->base.y);
                        }


                      /* Get a distro from a brick? */

                      if (shape(pplayer->base.x, pplayer->base.y) == 'x' ||
                          shape(pplayer->base.x, pplayer->base.y) == 'y')
                        {
                          add_bouncy_distro(((pplayer->base.x+ 1)
                                             / 32) * 32,
                                            (int)(pplayer->base.y / 32) * 32);

                          if (counting_distros == NO)
                            {
                              counting_distros = YES;
                              distro_counter = 100;
                            }

                          if (distro_counter <= 0)
                            level_change(&current_level,pplayer->base.x,pplayer->base.y, 'a');

                          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                          score = score + SCORE_DISTRO;
                          distros++;
                        }
                      else if (shape(pplayer->base.x+ 31, pplayer->base.y) == 'x' ||
                               shape(pplayer->base.x+ 31, pplayer->base.y) == 'y')
                        {
                          add_bouncy_distro(((pplayer->base.x+ 1 + 31)
                                             / 32) * 32,
                                            (int)(pplayer->base.y / 32) * 32);

                          if (counting_distros == NO)
                            {
                              counting_distros = YES;
                              distro_counter = 100;
                            }

                          if (distro_counter <= 0)
                            level_change(&current_level,pplayer->base.x+ 31 + scroll_x, pplayer->base.y, 'a');

                          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
                          score = score + SCORE_DISTRO;
                          distros++;
                        }
                    }


                  /* Bump head: */

                  pplayer->base.y = (int)(pplayer->base.y / 32) * 32 + 30;
                }
              else
                {
                  /* Land on feet: */

                  pplayer->base.y = (int)(pplayer->base.y / 32) * 32 - 32;
                }

              pplayer->base.ym = 0;
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
          if (isice(pplayer->base.x, pplayer->base.y + 32) ||
              !issolid(pplayer->base.x, pplayer->base.y + 32))
            {
              /* Slowly on ice or in air: */

              if (pplayer->base.xm > 0)
                pplayer->base.xm--;
              else if (pplayer->base.xm < 0)
                pplayer->base.xm++;
            }
          else
            {
              /* Quickly, otherwise: */

              pplayer->base.xm = pplayer->base.xm / 2;
            }
        }


      /* Drop vertically: */

      if (!issolid(pplayer->base.x, pplayer->base.y + 32))
        {
          pplayer->base.ym = pplayer->base.ym + GRAVITY;

          if (pplayer->base.ym > MAX_YM)
            pplayer->base.ym = MAX_YM;
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

  if (pplayer->base.x>= endpos && endpos != 0)
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
          if (pplayer->base.xm < -SKID_XM && !pplayer->skidding &&
              pplayer->dir == LEFT)
            {
              pplayer->skidding = SKID_TIME;

              play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);

            }
          pplayer->dir = RIGHT;
        }

      if (pplayer->base.xm < 0 && !isice(pplayer->base.x, pplayer->base.y + 32) &&
          !pplayer->skidding)
        {
          pplayer->base.xm = 0;
        }

      if (!pplayer->duck)
        {
          if (pplayer->dir == RIGHT)
            {
              /* Facing the direction we're jumping?  Go full-speed: */

              if (pplayer->input.fire == UP)
                {
                  pplayer->base.xm = pplayer->base.xm + WALK_SPEED;

                  if (pplayer->base.xm > MAX_WALK_XM)
                    pplayer->base.xm = MAX_WALK_XM;
                }
              else if ( pplayer->input.fire == DOWN)
                {
                  pplayer->base.xm = pplayer->base.xm + RUN_SPEED;

                  if (pplayer->base.xm > MAX_RUN_XM)
                    pplayer->base.xm = MAX_RUN_XM;
                }
            }
          else
            {
              /* Not facing the direction we're jumping?
              Go half-speed: */

              pplayer->base.xm = pplayer->base.xm + WALK_SPEED / 2;

              if (pplayer->base.xm > MAX_WALK_XM / 2)
                pplayer->base.xm = MAX_WALK_XM / 2;
            }
        }
    }
  else if (pplayer->input.left == DOWN && pplayer->input.right == UP)
    {
      if (pplayer->jumping == NO)
        {
          if (pplayer->base.xm > SKID_XM && !pplayer->skidding &&
              pplayer->dir == RIGHT)
            {
              pplayer->skidding = SKID_TIME;
              play_sound(sounds[SND_SKID], SOUND_CENTER_SPEAKER);
            }
          pplayer->dir = LEFT;
        }

      if (pplayer->base.xm > 0 && !isice(pplayer->base.x, pplayer->base.y + 32) &&
          !pplayer->skidding)
        {
          pplayer->base.xm = 0;
        }

      if (!pplayer->duck)
        {
          if (pplayer->dir == LEFT)
            {
              /* Facing the direction we're jumping?  Go full-speed: */

              if (pplayer->input.fire == UP)
                {
                  pplayer->base.xm = pplayer->base.xm - WALK_SPEED;

                  if (pplayer->base.xm < -MAX_WALK_XM)
                    pplayer->base.xm = -MAX_WALK_XM;
                }
              else if (pplayer->input.fire == DOWN)
                {
                  pplayer->base.xm = pplayer->base.xm - RUN_SPEED;

                  if (pplayer->base.xm < -MAX_RUN_XM)
                    pplayer->base.xm = -MAX_RUN_XM;
                }
            }
          else
            {
              /* Not facing the direction we're jumping?
              Go half-speed: */

              pplayer->base.xm = pplayer->base.xm - WALK_SPEED / 2;

              if (pplayer->base.xm < -MAX_WALK_XM / 2)
                pplayer->base.xm = -MAX_WALK_XM / 2;
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

          if (!issolid(pplayer->base.x, pplayer->base.y + 32) ||
              pplayer->base.ym != 0)
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
                  !issolid(pplayer->base.x, pplayer->base.y))
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
          pplayer->base.ym = pplayer->base.ym - JUMP_SPEED;
        }
    }
  else
    timer_stop(&pplayer->jump_timer);


  /* Shoot! */

  if (pplayer->input.fire == DOWN && pplayer->input.old_fire == UP && pplayer->got_coffee)
    {
      add_bullet(pplayer->base.x, pplayer->base.y, pplayer->base.xm, pplayer->dir);
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

          if (!issolid(pplayer->base.x, pplayer->base.y - 32))
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
      trygrabdistro(pplayer->base.x, pplayer->base.y, NO_BOUNCE);
      trygrabdistro(pplayer->base.x+ 31, pplayer->base.y, NO_BOUNCE);

      if (pplayer->size == BIG && !pplayer->duck)
        {
          trygrabdistro(pplayer->base.x, pplayer->base.y - 32, NO_BOUNCE);
          trygrabdistro(pplayer->base.x+ 31, pplayer->base.y - 32, NO_BOUNCE);
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
                               pplayer->base.x- scroll_x - 8 - 16, pplayer->base.y - 32,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&bigcape_left[frame % 2],
                               pplayer->base.x-scroll_x - 8, pplayer->base.y - 32,
                               NO_UPDATE);
                }
            }

          if (!pplayer->got_coffee)
            {
              if (!pplayer->duck)
                {
                  if (!pplayer->skidding)
                    {
                      if (!pplayer->jumping || pplayer->base.ym > 0)
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigtux_right[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigtux_left[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                        }
                      else
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigtux_right_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigtux_left_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                        }
                    }
                  else
                    {
                      if (pplayer->dir == RIGHT)
                        {
                          texture_draw(&skidtux_right,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                       NO_UPDATE);
                        }
                      else
                        {
                          texture_draw(&skidtux_left,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
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
                  if (!pplayer->skidding)
                    {
                      if (!pplayer->jumping || pplayer->base.ym > 0)
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left[pplayer->frame],
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                        }
                      else
                        {
                          if (pplayer->dir == RIGHT)
                            {
                              texture_draw(&bigfiretux_right_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                          else
                            {
                              texture_draw(&bigfiretux_left_jump,
                                           pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                           NO_UPDATE);
                            }
                        }
                    }
                  else
                    {
                      if (pplayer->dir == RIGHT)
                        {
                          texture_draw(&skidfiretux_right,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
                                       NO_UPDATE);
                        }
                      else
                        {
                          texture_draw(&skidfiretux_left,
                                       pplayer->base.x- scroll_x - 8, pplayer->base.y - 32,
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

              if (pplayer->base.x<= pbad_c->base.x)
                {
                  pbad_c->dir = RIGHT;
                  pbad_c->base.x = pbad_c->base.x + 16;
                }
              else
                {
                  pbad_c->dir = LEFT;
                  pbad_c->base.x = pbad_c->base.x - 16;
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
                          pbad_c->base.ym = -8;
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
                  pbad_c->base.ym = -8;
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
  pplayer->base.ym = -5;

  play_sound(sounds[SND_HURT], SOUND_CENTER_SPEAKER);

  if (pplayer->dir == RIGHT)
    pplayer->base.xm = -8;
  else if (tux.dir == LEFT)
    pplayer->base.xm = 8;

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
  pplayer->base.ym = pplayer->base.ym + GRAVITY;

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

  if (pplayer->base.y < 0)
    pplayer->base.y = 0;
  else if (pplayer->base.y > screen->h)
    {
      player_kill(&tux,KILL);
    }
}
