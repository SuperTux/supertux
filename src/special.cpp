//
// C Implementation: special
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de> & Bill Kendrick, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "SDL.h"
#include "defines.h"
#include "special.h"
#include "gameloop.h"
#include "screen.h"
#include "sound.h"
#include "scene.h"
#include "globals.h"
#include "player.h"

texture_type img_bullet;
texture_type img_golden_herring;
bitmask* bm_bullet;

void create_special_bitmasks()
{
  bm_bullet = bitmask_create_SDL(img_bullet.sdl_surface);
}

void bullet_init(bullet_type* pbullet, float x, float y, float xm, int dir)
{
  pbullet->base.width = 4;
  pbullet->base.height = 4;
  pbullet->base.alive = YES;

  if (dir == RIGHT)
    {
      pbullet->base.x = x + 32;
      pbullet->base.xm = BULLET_XM + xm;
    }
  else
    {
      pbullet->base.x = x;
      pbullet->base.xm = -BULLET_XM + xm;
    }

  pbullet->base.y = y;
  pbullet->base.ym = BULLET_STARTING_YM;
  pbullet->old_base = pbullet->base;
}

void bullet_action(bullet_type* pbullet)
{
  if (pbullet->base.alive)
    {
      pbullet->base.x = pbullet->base.x + pbullet->base.xm * frame_ratio;
      pbullet->base.y = pbullet->base.y + pbullet->base.ym * frame_ratio;

      collision_swept_object_map(&pbullet->old_base,&pbullet->base);
      
      if (issolid(pbullet->base.x, pbullet->base.y + 4) || issolid(pbullet->base.x, pbullet->base.y))
        {
              pbullet->base.ym = -pbullet->base.ym;
	      pbullet->base.y = (int)(pbullet->base.y / 32) * 32;
        }

      pbullet->base.ym = pbullet->base.ym + GRAVITY;

      if (pbullet->base.x < scroll_x ||
          pbullet->base.x > scroll_x + screen->w ||
	  pbullet->base.y < 0 ||
	  pbullet->base.y > screen->h ||
	  issolid(pbullet->base.x + 4, pbullet->base.y + 2) ||
	  issolid(pbullet->base.x, pbullet->base.y + 2))
        {
          pbullet->base.alive = NO;
        }
    }

}

void bullet_draw(bullet_type* pbullet)
{
  if (pbullet->base.alive  &&
      pbullet->base.x >= scroll_x - pbullet->base.width &&
      pbullet->base.x <= scroll_x + screen->w)
    {
      texture_draw(&img_bullet, pbullet->base.x - scroll_x, pbullet->base.y,
                   NO_UPDATE);
    }
}

void bullet_collision(bullet_type* pbullet, int c_object)
{

  if(c_object == CO_BADGUY)
    pbullet->base.alive = NO;

}

void upgrade_init(upgrade_type *pupgrade, float x, float y, int dir, int kind)
{
  pupgrade->base.width = 32;
  pupgrade->base.height = 0;
  pupgrade->base.alive = YES;
  pupgrade->kind = kind;
  pupgrade->base.x = x;
  pupgrade->base.y = y;
  if(dir == LEFT)
    pupgrade->base.xm = -2;
  else
    pupgrade->base.xm = 2;
  pupgrade->base.ym = -2;
  pupgrade->base.height = 0;
  pupgrade->old_base = pupgrade->base;
}

void upgrade_action(upgrade_type *pupgrade)
{

  if (pupgrade->base.alive)
    {
      if (pupgrade->base.height < 32)
        {
          /* Rise up! */

          pupgrade->base.height = pupgrade->base.height + 0.7 * frame_ratio;
          if(pupgrade->base.height > 32)
            pupgrade->base.height = 32;
        }
      else
        {
          /* Move around? */

          if (pupgrade->kind == UPGRADE_MINTS ||
              pupgrade->kind == UPGRADE_HERRING)
            {
              pupgrade->base.x = pupgrade->base.x + pupgrade->base.xm * frame_ratio;
              pupgrade->base.y = pupgrade->base.y + pupgrade->base.ym * frame_ratio;

              collision_swept_object_map(&pupgrade->old_base,&pupgrade->base);

              /* Off the screen?  Kill it! */

              if (pupgrade->base.x < scroll_x - pupgrade->base.width)
                pupgrade->base.alive = NO;
              if (pupgrade->base.y > screen->h)
                pupgrade->base.alive = NO;

              if (issolid(pupgrade->base.x + 1, pupgrade->base.y + 32.) ||
                  issolid(pupgrade->base.x + 31., pupgrade->base.y + 32.))
                {
                  if (pupgrade->base.ym > 0)
                    {
                      if (pupgrade->kind == UPGRADE_MINTS)
                        {
                          pupgrade->base.ym = 0;
                        }
                      else if (pupgrade->kind == UPGRADE_HERRING)
                        {
                          pupgrade->base.ym = -8;
                        }

                      pupgrade->base.y = (int)(pupgrade->base.y / 32) * 32;
                    }
                }
              else
                pupgrade->base.ym = pupgrade->base.ym + GRAVITY * frame_ratio;

              if (issolid(pupgrade->base.x - 1, (int) pupgrade->base.y))
                {
                  if(pupgrade->base.xm < 0)
                    pupgrade->base.xm = -pupgrade->base.xm;
                }
              else if (issolid(pupgrade->base.x + pupgrade->base.width, (int) pupgrade->base.y))
                {
                  if(pupgrade->base.xm > 0)
                    pupgrade->base.xm = -pupgrade->base.xm;
                }
            }

        }
    }
}

void upgrade_draw(upgrade_type* pupgrade)
{
  SDL_Rect dest;
  if (pupgrade->base.alive)
    {
      if (pupgrade->base.height < 32)
        {
          /* Rising up... */

          dest.x = (int)(pupgrade->base.x - scroll_x);
          dest.y = (int)(pupgrade->base.y + 32 - pupgrade->base.height);
          dest.w = 32;
          dest.h = (int)pupgrade->base.height;

          if (pupgrade->kind == UPGRADE_MINTS)
            texture_draw_part(&img_mints,0,0,dest.x,dest.y,dest.w,dest.h,NO_UPDATE);
          else if (pupgrade->kind == UPGRADE_COFFEE)
            texture_draw_part(&img_coffee,0,0,dest.x,dest.y,dest.w,dest.h,NO_UPDATE);
          else if (pupgrade->kind == UPGRADE_HERRING)
            texture_draw_part(&img_golden_herring,0,0,dest.x,dest.y,dest.w,dest.h,NO_UPDATE);
        }
      else
        {
          if (pupgrade->kind == UPGRADE_MINTS)
            {
              texture_draw(&img_mints,
                           pupgrade->base.x - scroll_x, pupgrade->base.y,
                           NO_UPDATE);
            }
          else if (pupgrade->kind == UPGRADE_COFFEE)
            {
              texture_draw(&img_coffee,
                           pupgrade->base.x - scroll_x, pupgrade->base.y,
                           NO_UPDATE);
            }
          else if (pupgrade->kind == UPGRADE_HERRING)
            {
              texture_draw(&img_golden_herring,
                           pupgrade->base.x - scroll_x, pupgrade->base.y,
                           NO_UPDATE);
            }
        }
    }
}

void upgrade_collision(upgrade_type* pupgrade, void* p_c_object, int c_object)
{
  player_type* pplayer = NULL;

  switch (c_object)
    {
    case CO_PLAYER:
      /* Remove the upgrade: */

      /* p_c_object is CO_PLAYER, so assign it to pplayer */
      pplayer = (player_type*) p_c_object;

      pupgrade->base.alive = NO;

      /* Affect the player: */

      if (pupgrade->kind == UPGRADE_MINTS)
        {
          play_sound(sounds[SND_EXCELLENT], SOUND_CENTER_SPEAKER);
          pplayer->size = BIG;
          pplayer->base.height = 64;
	  pplayer->base.y -= 32;
	  if(collision_object_map(&pplayer->base))
	  {
	  pplayer->base.height = 32;
	  pplayer->base.y += 32;
	  pplayer->duck = YES;
	  }
          timer_start(&super_bkgd_timer, 350);
        }
      else if (pupgrade->kind == UPGRADE_COFFEE)
        {
          play_sound(sounds[SND_COFFEE], SOUND_CENTER_SPEAKER);
          pplayer->got_coffee = YES;
          timer_start(&super_bkgd_timer, 250);
        }
      else if (pupgrade->kind == UPGRADE_HERRING)
        {
          play_sound(sounds[SND_HERRING], SOUND_CENTER_SPEAKER);
          timer_start(&pplayer->invincible_timer,TUX_INVINCIBLE_TIME);
          timer_start(&super_bkgd_timer, 250);
          /* play the herring song ^^ */
          if (get_current_music() != HURRYUP_MUSIC)
            {
              set_current_music(HERRING_MUSIC);
              play_current_music();
            }
        }
      break;
    }
}

