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
#include "resources.h"

texture_type img_bullet;
texture_type img_golden_herring;
bitmask* bm_bullet;

void create_special_bitmasks()
{
  bm_bullet = bitmask_create_SDL(img_bullet.sdl_surface);
}

void
Bullet::init(float x, float y, float xm, int dir)
{
  base.width = 4;
  base.height = 4;

  if (dir == RIGHT)
    {
      base.x = x + 32;
      base.xm = BULLET_XM + xm;
    }
  else
    {
      base.x = x;
      base.xm = -BULLET_XM + xm;
    }

  base.y = y;
  base.ym = BULLET_STARTING_YM;
  old_base = base;
}

void
Bullet::action()
{
  base.x = base.x + base.xm * frame_ratio;
  base.y = base.y + base.ym * frame_ratio;

  collision_swept_object_map(&old_base,&base);
      
  if (issolid(base.x, base.y + 4) || issolid(base.x, base.y))
    {
      base.ym = -base.ym;
      base.y = (int)(base.y / 32) * 32;
    }

  base.ym = base.ym + GRAVITY;

  if (base.x < scroll_x ||
      base.x > scroll_x + screen->w ||
      base.y < 0 ||
      base.y > screen->h ||
      issolid(base.x + 4, base.y + 2) ||
      issolid(base.x, base.y + 2))
    {
      World::current()->bullets.erase(static_cast<std::vector<Bullet>::iterator>(this));
    }

}

void 
Bullet::draw()
{
  if (base.x >= scroll_x - base.width &&
      base.x <= scroll_x + screen->w)
    {
      texture_draw(&img_bullet, base.x - scroll_x, base.y, 255,
                   NO_UPDATE);
    }
}

void
Bullet::collision(int c_object)
{
  if(c_object == CO_BADGUY) {
    std::vector<Bullet>::iterator i;
    
    for(i = World::current()->bullets.begin(); i != World::current()->bullets.end(); ++i) 
      {
        if(&(*i) == this) 
          {
            World::current()->bullets.erase(i);
            return;
          }
      }
  }
}

void
Upgrade::init(float x_, float y_, int dir_, int kind_)
{
  base.width = 32;
  base.height = 0;
  kind = kind_;
  base.x = x_;
  base.y = y_;

  if(dir_ == LEFT)
    base.xm = -2;
  else
    base.xm = 2;

  base.ym = -2;
  base.height = 0;
  old_base = base;
}

void
Upgrade::action()
{
  if (base.height < 32)
    {
      /* Rise up! */

      base.height = base.height + 0.7 * frame_ratio;
      if(base.height > 32)
        base.height = 32;
    }
  else
    {
      /* Move around? */

      if (kind == UPGRADE_MINTS ||
          kind == UPGRADE_HERRING)
        {
          base.x = base.x + base.xm * frame_ratio;
          base.y = base.y + base.ym * frame_ratio;

          collision_swept_object_map(&old_base,&base);

          /* Off the screen?  Kill it! */

          if (base.x < scroll_x - base.width)
            World::current()->upgrades.erase(static_cast<std::vector<Upgrade>::iterator>(this));
          if (base.y > screen->h)
            World::current()->upgrades.erase(static_cast<std::vector<Upgrade>::iterator>(this));

          if (issolid(base.x + 1, base.y + 32.) ||
              issolid(base.x + 31., base.y + 32.))
            {
              if (base.ym > 0)
                {
                  if (kind == UPGRADE_MINTS)
                    {
                      base.ym = 0;
                    }
                  else if (kind == UPGRADE_HERRING)
                    {
                      base.ym = -8;
                    }

                  base.y = (int)(base.y / 32) * 32;
                }
            }
          else
            base.ym = base.ym + GRAVITY * frame_ratio;

          if (issolid(base.x - 1, (int) base.y))
            {
              if(base.xm < 0)
                base.xm = -base.xm;
            }
          else if (issolid(base.x + base.width, (int) base.y))
            {
              if(base.xm > 0)
                base.xm = -base.xm;
            }
        }

    }
}

void
Upgrade::draw()
{
  SDL_Rect dest;
  if (base.height < 32)
    {
      /* Rising up... */

      dest.x = (int)(base.x - scroll_x);
      dest.y = (int)(base.y + 32 - base.height);
      dest.w = 32;
      dest.h = (int)base.height;

      if (kind == UPGRADE_MINTS)
        texture_draw_part(&img_mints,0,0,dest.x,dest.y,dest.w,dest.h);
      else if (kind == UPGRADE_COFFEE)
        texture_draw_part(&img_coffee,0,0,dest.x,dest.y,dest.w,dest.h);
      else if (kind == UPGRADE_HERRING)
        texture_draw_part(&img_golden_herring,0,0,dest.x,dest.y,dest.w,dest.h);
    }
  else
    {
      if (kind == UPGRADE_MINTS)
        {
          texture_draw(&img_mints,
                       base.x - scroll_x, base.y);
        }
      else if (kind == UPGRADE_COFFEE)
        {
          texture_draw(&img_coffee,
                       base.x - scroll_x, base.y);
        }
      else if (kind == UPGRADE_HERRING)
        {
          texture_draw(&img_golden_herring,
                       base.x - scroll_x, base.y);
        }
    }
}

void
Upgrade::collision(void* p_c_object, int c_object)
{
  Player* pplayer = NULL;

  switch (c_object)
    {
    case CO_PLAYER:
      /* Remove the upgrade: */

      /* p_c_object is CO_PLAYER, so assign it to pplayer */
      pplayer = (Player*) p_c_object;

      World::current()->upgrades.erase(static_cast<std::vector<Upgrade>::iterator>(this));

      /* Affect the player: */

      if (kind == UPGRADE_MINTS)
        {
          play_sound(sounds[SND_EXCELLENT], SOUND_CENTER_SPEAKER);
          pplayer->size = BIG;
          pplayer->base.height = 64;
	  pplayer->base.y -= 32;
	  if(collision_object_map(&pplayer->base))
            {
              pplayer->base.height = 32;
              pplayer->base.y += 32;
              pplayer->duck = true;
            }
        }
      else if (kind == UPGRADE_COFFEE)
        {
          play_sound(sounds[SND_COFFEE], SOUND_CENTER_SPEAKER);
          pplayer->got_coffee = true;
        }
      else if (kind == UPGRADE_HERRING)
        {
          play_sound(sounds[SND_HERRING], SOUND_CENTER_SPEAKER);
          timer_start(&pplayer->invincible_timer,TUX_INVINCIBLE_TIME);
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

