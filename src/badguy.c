//
// C Implementation: badguy
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de> & Bill Kendrick, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "globals.h"
#include "defines.h"
#include "badguy.h"
#include "scene.h"
#include "screen.h"

texture_type img_bsod_squished_left, img_bsod_squished_right,
img_bsod_falling_left, img_bsod_falling_right,
img_laptop_flat_left, img_laptop_flat_right,
img_laptop_falling_left, img_laptop_falling_right;
texture_type img_bsod_left[4], img_bsod_right[4],
img_laptop_left[3], img_laptop_right[3],
img_money_left[2], img_money_right[2];
bitmask *bm_bsod;

void badguy_create_bitmasks()
{
  /*bm_bsod = img_bsod_left[0];*/
}

void badguy_init(bad_guy_type* pbad, float x, float y, int kind)
{
  pbad->base.width = 32;
  pbad->base.height = 32;
  pbad->base.alive = YES;
  pbad->mode = NORMAL;
  pbad->dying = NO;
  pbad->kind = kind;
  pbad->base.x = x;
  pbad->base.y = y;
  pbad->base.xm = 1.3;
  pbad->base.ym = 4.8;
  pbad->old_base = pbad->base;
  pbad->dir = LEFT;
  pbad->seen = NO;
  timer_init(&pbad->timer,YES);
  physic_init(&pbad->physic);
}

void badguy_action(bad_guy_type* pbad)
{

  if (pbad->base.alive)
    {
      if (pbad->seen)
        {
          if (pbad->kind == BAD_BSOD)
            {
              /* --- BLUE SCREEN OF DEATH MONSTER: --- */

              /* Move left/right: */

              if (pbad->dying == NO ||
                  pbad->dying == FALLING)
                {
                  if (pbad->dir == RIGHT)
                    pbad->base.x = pbad->base.x + pbad->base.xm * frame_ratio;
                  else if (pbad->dir == LEFT)
                    pbad->base.x = pbad->base.x - pbad->base.xm * frame_ratio;
                }


              /* Move vertically: */

              pbad->base.y = pbad->base.y + pbad->base.ym * frame_ratio;

              if (pbad->dying != FALLING)
                collision_swept_object_map(&pbad->old_base,&pbad->base);
              if (pbad->base.y > screen->h)
                pbad->base.alive = NO;

              /* Bump into things horizontally: */

              if (!pbad->dying)
                {
                  if (issolid( pbad->base.x, (int) pbad->base.y + 16))
                    {
                      pbad->dir = RIGHT;
                    }
                  else if (issolid( pbad->base.x + pbad->base.width, (int) pbad->base.y + 16))
                    {
                      pbad->dir = LEFT;
                    }
                }

              /* Fall if we get off the ground: */

              if (pbad->dying != FALLING)
                {
                  if (!issolid(pbad->base.x+16, pbad->base.y + 32))
                    {
                      if(!physic_is_set(&pbad->physic))
                        {
                          physic_set_state(&pbad->physic,PH_VT);
                          physic_set_start_vy(&pbad->physic,2.);
                        }

                      pbad->base.ym = physic_get_velocity(&pbad->physic);
                    }
                  else
                    {
                      /* Land: */

                      if (pbad->base.ym > 0)
                        {
                          pbad->base.y = (int)(pbad->base.y / 32) * 32;
                          pbad->base.ym = 0;
                        }
                      physic_init(&pbad->physic);
                    }
                }
              else
                {
                  if(!physic_is_set(&pbad->physic))
                    {
                      physic_set_state(&pbad->physic,PH_VT);
                      physic_set_start_vy(&pbad->physic,2.);
                    }
                  pbad->base.ym = physic_get_velocity(&pbad->physic);
                }

              if (pbad->base.y > screen->h)
                pbad->base.alive = NO;
            }
          else if (pbad->kind == BAD_LAPTOP)
            {
              /* --- LAPTOP MONSTER: --- */

              /* Move left/right: */

              if (pbad->mode == NORMAL || pbad->mode == KICK)
                {
                  if (pbad->dying == NO ||
                      pbad->dying == FALLING)
                    {
                      if (pbad->dir == RIGHT)
                        pbad->base.x = pbad->base.x + pbad->base.xm * frame_ratio;
                      else if (pbad->dir == LEFT)
                        pbad->base.x = pbad->base.x - pbad->base.xm * frame_ratio;
                    }
                }
              else if (pbad->mode == HELD)
                { /* FIXME: The pbad object shouldn't know about pplayer objects. */
                  /* If we're holding the laptop */
                  pbad->dir=tux.dir;
                  if(pbad->dir==RIGHT)
                    {
                      pbad->base.x = tux.base.x + 16;
                      pbad->base.y = tux.base.y + tux.base.height/1.5 - pbad->base.height;
                    }
                  else /* facing left */
                    {
                      pbad->base.x = tux.base.x - 16;
                      pbad->base.y = tux.base.y + tux.base.height/1.5 - pbad->base.height;
                    }
                  if(collision_object_map(&pbad->base))
                    {
                      pbad->base.x = tux.base.x;
                      pbad->base.y = tux.base.y + tux.base.height/1.5 - pbad->base.height;
                    }

                  if(tux.input.fire != DOWN) /* SHOOT! */
                    {
                      if(pbad->dir == LEFT)
                        pbad->base.x -= 24;
                      else
                        pbad->base.x += 24;

                      pbad->mode=KICK;
                      pbad->base.ym-=8;
                      play_sound(sounds[SND_KICK],SOUND_CENTER_SPEAKER);
                    }
                }


              /* Move vertically: */

              if(pbad->mode != HELD)
                pbad->base.y = pbad->base.y + pbad->base.ym * frame_ratio;

              if (pbad->dying != FALLING)
                collision_swept_object_map(&pbad->old_base,&pbad->base);
              if (pbad->base.y > screen->h)
                pbad->base.alive = NO;
              /* Bump into things horizontally: */

              /* Bump into things horizontally: */

              if (!pbad->dying)
                {
                  int changed = pbad->dir;
                  if (issolid( pbad->base.x, (int) pbad->base.y + 16))
                    {
                      pbad->dir = RIGHT;
                    }
                  else if (issolid( pbad->base.x + pbad->base.width, (int) pbad->base.y + 16))
                    {
                      pbad->dir = LEFT;
                    }
                  if(pbad->mode == KICK && changed != pbad->dir)
                    {
                      /* handle stereo sound */
                      /* FIXME: In theory a badguy object doesn't know anything about player objects */
                      if (tux.base.x  > pbad->base.x)
                        play_sound(sounds[SND_RICOCHET], SOUND_LEFT_SPEAKER);
                      else if (tux.base.x  < pbad->base.x)
                        play_sound(sounds[SND_RICOCHET], SOUND_RIGHT_SPEAKER);
                      else
                        play_sound(sounds[SND_RICOCHET], SOUND_CENTER_SPEAKER);
                    }

                }


              /* Fall if we get off the ground: */

              if (pbad->dying != FALLING)
                {
                  if (!issolid(pbad->base.x+16, pbad->base.y + 32))
                    {
                      if(!physic_is_set(&pbad->physic))
                        {
                          physic_set_state(&pbad->physic,PH_VT);
                          physic_set_start_vy(&pbad->physic,0.);
                        }

                      if(pbad->mode != HELD)
                        {
                          pbad->base.ym = physic_get_velocity(&pbad->physic);
                        }
                    }
                  else
                    {
                      /* Land: */

                      if (pbad->base.ym > 0)
                        {
                          pbad->base.y = (int)(pbad->base.y / 32) * 32;
                          pbad->base.ym = 0;
                        }
                      physic_init(&pbad->physic);
                    }
                }
              else
                {
                  if(!physic_is_set(&pbad->physic))
                    {
                      physic_set_state(&pbad->physic,PH_VT);
                      physic_set_start_vy(&pbad->physic,0.);
                    }
                  pbad->base.ym = physic_get_velocity(&pbad->physic);
                }


            }
          else if (pbad->kind == BAD_MONEY)
            {
              /* --- MONEY BAGS: --- */


              /* Move vertically: */

              pbad->base.y = pbad->base.y + pbad->base.ym * frame_ratio;

              if (pbad->dying != FALLING)
                collision_swept_object_map(&pbad->old_base,&pbad->base);

              if (pbad->base.y > screen->h)
                pbad->base.alive = NO;

              if(physic_get_state(&pbad->physic) == -1)
                {
                  physic_set_state(&pbad->physic,PH_VT);
                  physic_set_start_vy(&pbad->physic,0.);
                }

              if (pbad->dying != FALLING)
                {
                  if(issolid(pbad->base.x, pbad->base.y + 32))
                    {
                      physic_set_state(&pbad->physic,PH_VT);
                      physic_set_start_vy(&pbad->physic,6.);
                      pbad->base.ym = physic_get_velocity(&pbad->physic);
                    }
                  else if(issolid(pbad->base.x, pbad->base.y))
                    { /* This works, but isn't the best solution imagineable */
                      physic_set_state(&pbad->physic,PH_VT);
                      physic_set_start_vy(&pbad->physic,0.);
                      pbad->base.ym = physic_get_velocity(&pbad->physic);
                      ++pbad->base.y;
                    }
                  else
                    {
                      pbad->base.ym = physic_get_velocity(&pbad->physic);
                    }
                }
              else
                {
                  if(!physic_is_set(&pbad->physic))
                    {
                      physic_set_state(&pbad->physic,PH_VT);
                      physic_set_start_vy(&pbad->physic,0.);
                    }
                  pbad->base.ym = physic_get_velocity(&pbad->physic);
                }
            }
          else if (pbad->kind == -1)
          {}

        }
      else if (pbad->kind == -1)
      {}


    }

  /* Handle mode timer: */

  if (pbad->mode == FLAT && pbad->mode != HELD)
    {
      if(!timer_check(&pbad->timer))
      {
        pbad->mode = NORMAL;
        pbad->base.xm = 4;
        }
    }
  else if (pbad->mode == KICK)
    {
      timer_check(&pbad->timer);
    }


  /* Handle dying timer: */

  if (pbad->dying == SQUISHED)
    {
      /* Remove it if time's up: */
      if(!timer_check(&pbad->timer))
        pbad->base.alive = NO;
    }


  /* Remove if it's far off the screen: */

  if (pbad->base.x < scroll_x - OFFSCREEN_DISTANCE)
    pbad->base.alive = NO;
  else /* !seen */
    {
      /* Once it's on screen, it's activated! */

      if (pbad->base.x <= scroll_x + screen->w + OFFSCREEN_DISTANCE)
        pbad->seen = YES;
    }
  /*}*/
}

void badguy_draw(bad_guy_type* pbad)
{
  if (pbad->base.alive &&
      pbad->base.x > scroll_x - 32 &&
      pbad->base.x < scroll_x + screen->w)
    {
      if (pbad->kind == BAD_BSOD)
        {
          /* --- BLUE SCREEN OF DEATH MONSTER: --- */

          if (pbad->dying == NO)
            {
              /* Alive: */

              if (pbad->dir == LEFT)
                {
                  texture_draw(&img_bsod_left[(frame / 5) % 4],
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&img_bsod_right[(frame / 5) % 4],
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
            }
          else if (pbad->dying == FALLING)
            {
              /* Falling: */

              if (pbad->dir == LEFT)
                {
                  texture_draw(&img_bsod_falling_left,
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&img_bsod_falling_right,
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
            }
          else if (pbad->dying == SQUISHED)
            {
              /* Dying - Squished: */

              if (pbad->dir == LEFT)
                {
                  texture_draw(&img_bsod_squished_left,
                               pbad->base.x - scroll_x,
                               pbad->base.y + 24,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&img_bsod_squished_right,
                               pbad->base.x - scroll_x,
                               pbad->base.y + 24,
                               NO_UPDATE);
                }
            }
        }
      else if (pbad->kind == BAD_LAPTOP)
        {
          /* --- LAPTOP MONSTER: --- */

          if (pbad->dying == NO)
            {
              /* Alive: */

              if (pbad->mode == NORMAL)
                {
                  /* Not flat: */

                  if (pbad->dir == LEFT)
                    {
                      texture_draw(&img_laptop_left[(frame / 5) % 3],
                                   pbad->base.x - scroll_x,
                                   pbad->base.y,
                                   NO_UPDATE);
                    }
                  else
                    {
                      texture_draw(&img_laptop_right[(frame / 5) % 3],
                                   pbad->base.x - scroll_x,
                                   pbad->base.y,
                                   NO_UPDATE);
                    }
                }
              else
                {
                  /* Flat: */

                  if (pbad->dir == LEFT)
                    {
                      texture_draw(&img_laptop_flat_left,
                                   pbad->base.x - scroll_x,
                                   pbad->base.y,
                                   NO_UPDATE);
                    }
                  else
                    {
                      texture_draw(&img_laptop_flat_right,
                                   pbad->base.x - scroll_x,
                                   pbad->base.y,
                                   NO_UPDATE);
                    }
                }
            }
          else if (pbad->dying == FALLING)
            {
              /* Falling: */

              if (pbad->dir == LEFT)
                {
                  texture_draw(&img_laptop_falling_left,
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&img_laptop_falling_right,
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
            }
        }
      else if (pbad->kind == BAD_MONEY)
        {
          if (pbad->base.ym != 300 /* > -16*/)
            {
              if (pbad->dir == LEFT)
                {
                  texture_draw(&img_money_left[0],
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&img_money_right[0],
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
            }
          else
            {
              if (pbad->dir == LEFT)
                {
                  texture_draw(&img_money_left[1],
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
              else
                {
                  texture_draw(&img_money_right[1],
                               pbad->base.x - scroll_x,
                               pbad->base.y,
                               NO_UPDATE);
                }
            }
        }
      else if (pbad->kind == -1)
      {}
    }
}

void badguy_collision(bad_guy_type* pbad, void *p_c_object, int c_object)
{
  bad_guy_type* pbad_c = NULL;
  player_type* pplayer_c = NULL;

  switch (c_object)
    {
    case CO_BULLET:
      pbad->dying = FALLING;
      pbad->base.ym = -8;

      /* Gain some points: */

      if (pbad->kind == BAD_BSOD)
        add_score(pbad->base.x - scroll_x, pbad->base.y,
                  50 * score_multiplier);
      else if (pbad->kind == BAD_LAPTOP)
        add_score(pbad->base.x - scroll_x, pbad->base.y,
                  25 * score_multiplier);
      else if (pbad->kind == BAD_MONEY)
        add_score(pbad->base.x - scroll_x, pbad->base.y,
                  50 * score_multiplier);

      /* Play death sound: */
      play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
      break;
    case CO_BADGUY:
      pbad_c = (bad_guy_type*) p_c_object;
      if (pbad->mode == NORMAL)
      {
      /* do nothing */
      }
      else if(pbad->mode == KICK)
        {
          /* We're in kick mode, kill the other guy
	     and yourself(wuahaha) : */

          pbad_c->dying = FALLING;
          pbad_c->base.ym = -8;
          play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);

          add_score(pbad->base.x - scroll_x,
                    pbad->base.y, 100);
	          pbad_c->dying = FALLING;
		  
          pbad->dying = FALLING;
          pbad->base.ym = -8;

          add_score(pbad_c->base.x - scroll_x,
                    pbad_c->base.y, 100);
        }
      break;
    case CO_PLAYER:
      pplayer_c = (player_type*) p_c_object;
      if(pbad->kind != BAD_MONEY)
        {
          if (pbad->kind == BAD_BSOD)
            {
              pbad->dying = SQUISHED;
              timer_start(&pbad->timer,4000);
              physic_set_state(&pplayer_c->vphysic,PH_VT);
              physic_set_start_vy(&pplayer_c->vphysic,2.);
	      pplayer_c->base.y = pbad->base.y - pplayer_c->base.height - 1;

              add_score(pbad->base.x - scroll_x, pbad->base.y,
                        50 * score_multiplier);

              play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
            }
          else if (pbad->kind == BAD_LAPTOP)
            {

              if (pbad->mode == NORMAL || pbad->mode == KICK)
                {
                  /* Flatten! */

                  play_sound(sounds[SND_STOMP], SOUND_CENTER_SPEAKER);
                  pbad->mode = FLAT;
                  pbad->base.xm = 4;

                  timer_start(&pbad->timer,10000);

                  physic_set_state(&pplayer_c->vphysic,PH_VT);
                  physic_set_start_vy(&pplayer_c->vphysic,2.);
		  pplayer_c->base.y = pbad->base.y - pplayer_c->base.height - 1;
                }
              else if (pbad->mode == FLAT)
                {
                  /* Kick! */
                  play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

                  if (pplayer_c->base.x <= pbad->base.x)
                    pbad->dir = RIGHT;
                  else
                    pbad->dir = LEFT;

                  pbad->base.xm = 5;
		  pbad->mode = KICK;

                  timer_start(&pbad->timer,5000);
                }
		
              physic_set_state(&pplayer_c->vphysic,PH_VT);
              physic_set_start_vy(&pplayer_c->vphysic,2.);
	      pplayer_c->base.y = pbad->base.y - pplayer_c->base.height - 1;
	      
              add_score(pbad->base.x - scroll_x,
                        pbad->base.y,
                        25 * score_multiplier);

              /* play_sound(sounds[SND_SQUISH]); */
            }
          score_multiplier++;
        }
      break;
    }

}
