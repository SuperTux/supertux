//
// C Implementation: special
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
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

void create_special_bitmasks()
{
  bm_bullet = bitmask_create_SDL(img_bullet.sdl_surface);
}

void bullet_init(bullet_type* pbullet, float x, float y, float xm, int dir)
{
  pbullet->base.width = 4;
  pbullet->base.height = 4;
  pbullet->base.updated = SDL_GetTicks();
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
}

void bullet_action(bullet_type* pbullet)
{

  double frame_ratio = get_frame_ratio(&pbullet->base);

  if (pbullet->base.alive)
    {
      pbullet->base.x = pbullet->base.x + pbullet->base.xm * frame_ratio;
      pbullet->base.y = pbullet->base.y + pbullet->base.ym * frame_ratio;

      if (issolid(pbullet->base.x, pbullet->base.y))
        {
          if (issolid(pbullet->base.x, pbullet->base.y - pbullet->base.ym))
            pbullet->base.alive = NO;
          else
            {
              if (pbullet->base.ym >= 0)
                {
                  pbullet->base.y = (int)(pbullet->base.y / 32) * 32 - 8;
                }
              pbullet->base.ym = -pbullet->base.ym;
            }
        }

      pbullet->base.ym = pbullet->base.ym + GRAVITY;

      if (pbullet->base.x < scroll_x ||
          pbullet->base.x > scroll_x + screen->w)
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

void upgrade_init(upgrade_type *pupgrade, float x, float y, int kind)
{
  pupgrade->base.width = 32;
  pupgrade->base.height = 0;
  pupgrade->base.alive = YES;
  pupgrade->kind = kind;
  pupgrade->base.x = x;
  pupgrade->base.y = y;
  pupgrade->base.xm = 2;
  pupgrade->base.ym = -2;
  pupgrade->base.height = 0;
  pupgrade->base.updated = SDL_GetTicks();
}

void upgrade_action(upgrade_type *pupgrade)
{
  double frame_ratio = get_frame_ratio(&pupgrade->base);

  if (pupgrade->base.alive)
    {
      if (pupgrade->base.height < 32)
        {
          /* Rise up! */

          pupgrade->base.height++;
        }
      else
        {
          /* Move around? */

          if (pupgrade->kind == UPGRADE_MINTS ||
              pupgrade->kind == UPGRADE_HERRING)
            {
              pupgrade->base.x = pupgrade->base.x + pupgrade->base.xm * frame_ratio;
              pupgrade->base.y = pupgrade->base.y + pupgrade->base.ym * frame_ratio;

              if (issolid(pupgrade->base.x, pupgrade->base.y + 31) ||
                  issolid(pupgrade->base.x + 31, pupgrade->base.y + 31))
                {
                  if (pupgrade->base.ym > 0)
                    {
                      if (pupgrade->kind == UPGRADE_MINTS)
                        {
                          pupgrade->base.ym = 0;
                        }
                      else if (pupgrade->kind == UPGRADE_HERRING)
                        {
                          pupgrade->base.ym = -24;
                        }

                      pupgrade->base.y = (int)(pupgrade->base.y / 32) * 32;
                    }
                }
              else
                pupgrade->base.ym = pupgrade->base.ym + GRAVITY;

              if (issolid(pupgrade->base.x, pupgrade->base.y))
                {
                  pupgrade->base.xm = -pupgrade->base.xm;
                }
            }


          /* Off the screen?  Kill it! */

          if (pupgrade->base.x < scroll_x - pupgrade->base.width)
            pupgrade->base.alive = NO;

        }
    }
}

void upgrade_draw(upgrade_type* pupgrade)
{
  if (pupgrade->base.alive)
    {
      if (pupgrade->base.height < 32)
        {
          /* Rising up... */

          dest.x = pupgrade->base.x - scroll_x;
          dest.y = pupgrade->base.y + 32 - pupgrade->base.height;
          dest.w = 32;
          dest.h = pupgrade->base.height;

          src.x = 0;
          src.y = 0;
          src.w = 32;
          src.h = pupgrade->base.height;

          if (pupgrade->kind == UPGRADE_MINTS)
            SDL_BlitSurface(img_mints.sdl_surface, &src, screen, &dest);
          else if (pupgrade->kind == UPGRADE_COFFEE)
            SDL_BlitSurface(img_coffee.sdl_surface, &src, screen, &dest);
          else if (pupgrade->kind == UPGRADE_HERRING)
            SDL_BlitSurface(img_golden_herring.sdl_surface, &src, screen, &dest);
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
      pplayer = p_c_object;

      pupgrade->base.alive = NO;

      /* Affect the player: */

      if (pupgrade->kind == UPGRADE_MINTS)
        {
          play_sound(sounds[SND_EXCELLENT], SOUND_CENTER_SPEAKER);
          pplayer->size = BIG;
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
          timer_start(&tux.invincible_timer,TUX_INVINCIBLE_TIME);
          timer_start(&super_bkgd_timer, 250);
          /* play the herring song ^^ */
          if (current_music != HURRYUP_MUSIC)
            {
              current_music = HERRING_MUSIC;
              if (playing_music())
                halt_music();
            }
        }
      break;
    }
}

