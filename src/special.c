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

void bullet_init(bullet_type* pbullet)
{
  pbullet->it.alive = &pbullet->alive;
  pbullet->it.x = &pbullet->x;
  pbullet->it.y = &pbullet->y;
  pbullet->it.width = &pbullet->width;
  pbullet->it.height = &pbullet->height;
  pbullet->it.updated = &pbullet->updated;

  pbullet->width = 4;
  pbullet->height = 4;
  pbullet->updated = SDL_GetTicks();
  pbullet->alive = NO;
}

void bullet_action(bullet_type* pbullet)
{

  double frame_ratio = get_frame_ratio(&pbullet->it);

  if (pbullet->alive)
    {
      pbullet->x = pbullet->x + pbullet->xm * frame_ratio;
      pbullet->y = pbullet->y + pbullet->ym * frame_ratio;

      if (issolid(pbullet->x, pbullet->y))
        {
          if (issolid(pbullet->x, pbullet->y - pbullet->ym))
            pbullet->alive = NO;
          else
            {
              if (pbullet->ym >= 0)
                {
                  pbullet->y = (int)(pbullet->y / 32) * 32 - 8;
                }
              pbullet->ym = -pbullet->ym;
            }
        }

      pbullet->ym = pbullet->ym + GRAVITY;

      if (pbullet->x < scroll_x ||
          pbullet->x > scroll_x + screen->w)
        {
          pbullet->alive = NO;
        }
    }

}

void bullet_draw(bullet_type* pbullet)
{
  if (pbullet->alive  &&
      pbullet->x >= scroll_x - pbullet->width &&
      pbullet->x <= scroll_x + screen->w)
    {
      texture_draw(&img_bullet, pbullet->x - scroll_x, pbullet->y,
                   NO_UPDATE);
    }
}

void bullet_collision(bullet_type* pbullet, int c_object)
{

  if(c_object == CO_BADGUY)
    pbullet->alive = NO;

}

void upgrade_init(upgrade_type *pupgrade)
{
  pupgrade->it.alive = &pupgrade->alive;
  pupgrade->it.x = &pupgrade->x;
  pupgrade->it.y = &pupgrade->y;
  pupgrade->it.width = &pupgrade->width;
  pupgrade->it.height = &pupgrade->height;
  pupgrade->it.updated = &pupgrade->updated;

  pupgrade->width = 32;
  pupgrade->height = 0;
  pupgrade->updated = SDL_GetTicks();
  pupgrade->alive = NO;
}

void upgrade_action(upgrade_type *pupgrade)
{
  double frame_ratio = get_frame_ratio(&pupgrade->it);

  if (pupgrade->alive)
    {
      if (pupgrade->height < 32)
        {
          /* Rise up! */

          pupgrade->height++;
        }
      else
        {
          /* Move around? */

          if (pupgrade->kind == UPGRADE_MINTS ||
              pupgrade->kind == UPGRADE_HERRING)
            {
              pupgrade->x = pupgrade->x + pupgrade->xm * frame_ratio;
              pupgrade->y = pupgrade->y + pupgrade->ym * frame_ratio;

              if (issolid(pupgrade->x, pupgrade->y + 31) ||
                  issolid(pupgrade->x + 31, pupgrade->y + 31))
                {
                  if (pupgrade->ym > 0)
                    {
                      if (pupgrade->kind == UPGRADE_MINTS)
                        {
                          pupgrade->ym = 0;
                        }
                      else if (pupgrade->kind == UPGRADE_HERRING)
                        {
                          pupgrade->ym = -24;
                        }

                      pupgrade->y = (int)(pupgrade->y / 32) * 32;
                    }
                }
              else
                pupgrade->ym = pupgrade->ym + GRAVITY;

              if (issolid(pupgrade->x, pupgrade->y))
                {
                  pupgrade->xm = -pupgrade->xm;
                }
            }


          /* Off the screen?  Kill it! */

          if (pupgrade->x < scroll_x)
            pupgrade->alive = NO;
        
	}
    }
}

void upgrade_draw(upgrade_type* pupgrade)
{
  if (pupgrade->alive)
    {
      if (pupgrade->height < 32)
        {
          /* Rising up... */

          dest.x = pupgrade->x - scroll_x;
          dest.y = pupgrade->y + 32 - pupgrade->height;
          dest.w = 32;
          dest.h = pupgrade->height;

          src.x = 0;
          src.y = 0;
          src.w = 32;
          src.h = pupgrade->height;

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
                           pupgrade->x - scroll_x, pupgrade->y,
                           NO_UPDATE);
            }
          else if (pupgrade->kind == UPGRADE_COFFEE)
            {
              texture_draw(&img_coffee,
                           pupgrade->x - scroll_x, pupgrade->y,
                           NO_UPDATE);
            }
          else if (pupgrade->kind == UPGRADE_HERRING)
            {
              texture_draw(&img_golden_herring,
                           pupgrade->x - scroll_x, pupgrade->y,
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
      
      pupgrade->alive = NO;

      /* Affect the player: */

      if (pupgrade->kind == UPGRADE_MINTS)
        {
          play_sound(sounds[SND_EXCELLENT], SOUND_CENTER_SPEAKER);
          pplayer->size = BIG;
          super_bkgd_time = 8;
        }
      else if (pupgrade->kind == UPGRADE_COFFEE)
        {
          play_sound(sounds[SND_COFFEE], SOUND_CENTER_SPEAKER);
          pplayer->got_coffee = YES;
          super_bkgd_time = 4;
        }
      else if (pupgrade->kind == UPGRADE_HERRING)
        {
          play_sound(sounds[SND_HERRING], SOUND_CENTER_SPEAKER);
          timer_start(&tux.invincible_timer,TUX_INVINCIBLE_TIME);
          super_bkgd_time = 4;
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

