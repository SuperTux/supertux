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

texture_type img_bsod_squished_left;
texture_type img_bsod_squished_right;
texture_type img_bsod_falling_left;
texture_type img_bsod_falling_right;
texture_type img_laptop_flat_left;
texture_type img_laptop_flat_right;
texture_type img_laptop_falling_left;
texture_type img_laptop_falling_right;
texture_type img_bsod_left[4];
texture_type img_bsod_right[4];
texture_type img_laptop_left[3];
texture_type img_laptop_right[3];
texture_type img_money_left[2];
texture_type img_money_right[2];

void
BadGuy::init(float x, float y, BadGuyKind kind_)
{
  base.width  = 32;
  base.height = 32;
  mode     = NORMAL;
  dying    = DYING_NOT;
  kind     = kind_;
  base.x   = x;
  base.y   = y;
  base.xm  = 1.3;
  base.ym  = 4.8;
  old_base = base;
  dir      = LEFT;
  seen     = false;
  timer_init(&timer, true);
  physic_init(&physic);
}

void BadGuy::action_bsod()
{
  /* --- BLUE SCREEN OF DEATH MONSTER: --- */

  /* Move left/right: */
  if (dying == DYING_NOT ||
      dying == DYING_FALLING)
    {
      if (dir == RIGHT)
        base.x = base.x + base.xm * frame_ratio;
      else if (dir == LEFT)
        base.x = base.x - base.xm * frame_ratio;
    }

  /* Move vertically: */
  base.y = base.y + base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);
  if (base.y > screen->h)
    bad_guys.erase(static_cast<std::vector<BadGuy>::iterator>(this));
		
  /* Bump into things horizontally: */

  if (!dying)
    {
      if (issolid( base.x, (int) base.y + 16))
        {
          dir = RIGHT;
        }
      else if (issolid( base.x + base.width, (int) base.y + 16))
        {
          dir = LEFT;
        }
    }

  /* Fall if we get off the ground: */
  if (dying != DYING_FALLING)
    {
      if (!issolid(base.x+16, base.y + 32))
        {
          if(!physic_is_set(&physic))
            {
              physic_set_state(&physic,PH_VT);
              physic_set_start_vy(&physic,2.);
            }

          base.ym = physic_get_velocity(&physic);
        }
      else
        {
          /* Land: */

          if (base.ym > 0)
            {
              base.y = (int)(base.y / 32) * 32;
              base.ym = 0;
            }
          physic_init(&physic);
        }
    }
  else
    {
      if(!physic_is_set(&physic))
        {
          physic_set_state(&physic,PH_VT);
          physic_set_start_vy(&physic,2.);
        }
      base.ym = physic_get_velocity(&physic);
    }

  // BadGuy fall below the ground
  if (base.y > screen->h)
    bad_guys.erase(static_cast<std::vector<BadGuy>::iterator>(this));
}

void BadGuy::action_laptop()
{
  /* Move left/right: */
  if (mode == NORMAL || mode == KICK)
    {
      if (dying == DYING_NOT ||
          dying == DYING_FALLING)
        {
          if (dir == RIGHT)
            base.x = base.x + base.xm * frame_ratio;
          else if (dir == LEFT)
            base.x = base.x - base.xm * frame_ratio;
        }
    }
  else if (mode == HELD)
    { /* FIXME: The pbad object shouldn't know about pplayer objects. */
      /* If we're holding the laptop */
      dir=tux.dir;
      if(dir==RIGHT)
        {
          base.x = tux.base.x + 16;
          base.y = tux.base.y + tux.base.height/1.5 - base.height;
        }
      else /* facing left */
        {
          base.x = tux.base.x - 16;
          base.y = tux.base.y + tux.base.height/1.5 - base.height;
        }
      if(collision_object_map(&base))
        {
          base.x = tux.base.x;
          base.y = tux.base.y + tux.base.height/1.5 - base.height;
        }

      if(tux.input_.fire != DOWN) /* SHOOT! */
        {
          if(dir == LEFT)
            base.x -= 24;
          else
            base.x += 24;

          mode=KICK;
          base.xm = 8;
          base.ym = 8;
          play_sound(sounds[SND_KICK],SOUND_CENTER_SPEAKER);
        }
    }


  /* Move vertically: */
  if(mode != HELD)
    base.y = base.y + base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);
  if (base.y > screen->h)
    bad_guys.erase(static_cast<std::vector<BadGuy>::iterator>(this));
  /* Bump into things horizontally: */

  /* Bump into things horizontally: */

  if (!dying)
    {
      int changed = dir;
      if (issolid( base.x, (int) base.y + 16))
        {
          dir = RIGHT;
        }
      else if (issolid( base.x + base.width, (int) base.y + 16))
        {
          dir = LEFT;
        }
      if(mode == KICK && changed != dir)
        {
          /* handle stereo sound */
          /* FIXME: In theory a badguy object doesn't know anything about player objects */
          if (tux.base.x  > base.x)
            play_sound(sounds[SND_RICOCHET], SOUND_LEFT_SPEAKER);
          else if (tux.base.x  < base.x)
            play_sound(sounds[SND_RICOCHET], SOUND_RIGHT_SPEAKER);
          else
            play_sound(sounds[SND_RICOCHET], SOUND_CENTER_SPEAKER);
        }

    }

  /* Fall if we get off the ground: */
  if (dying != DYING_FALLING)
    {
      if (!issolid(base.x+16, base.y + 32))
        {
          if(!physic_is_set(&physic))
            {
              physic_set_state(&physic,PH_VT);
              physic_set_start_vy(&physic,0.);
            }

          if(mode != HELD)
            {
              base.ym = physic_get_velocity(&physic);
            }
        }
      else
        {
          /* Land: */

          if (base.ym > 0)
            {
              base.y = (int)(base.y / 32) * 32;
              base.ym = 0;
            }
          physic_init(&physic);
        }
    }
  else
    {
      if(!physic_is_set(&physic))
        {
          physic_set_state(&physic,PH_VT);
          physic_set_start_vy(&physic,0.);
        }
      base.ym = physic_get_velocity(&physic);
    }
}

void BadGuy::action_money()
{
  /* Move vertically: */
  base.y = base.y + base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);

  if (base.y > screen->h)
    bad_guys.erase(static_cast<std::vector<BadGuy>::iterator>(this));

  if(physic_get_state(&physic) == -1)
    {
      physic_set_state(&physic,PH_VT);
      physic_set_start_vy(&physic,0.);
    }

  if (dying != DYING_FALLING)
    {
      if(issolid(base.x, base.y + 32))
        {
          physic_set_state(&physic,PH_VT);
          physic_set_start_vy(&physic,6.);
          base.ym = physic_get_velocity(&physic);
        }
      else if(issolid(base.x, base.y))
        { /* This works, but isn't the best solution imagineable */
          physic_set_state(&physic,PH_VT);
          physic_set_start_vy(&physic,0.);
          base.ym = physic_get_velocity(&physic);
          ++base.y;
        }
      else
        {
          base.ym = physic_get_velocity(&physic);
        }
    }
  else
    {
      if(!physic_is_set(&physic))
        {
          physic_set_state(&physic,PH_VT);
          physic_set_start_vy(&physic,0.);
        }
      base.ym = physic_get_velocity(&physic);
    } 
}

void
BadGuy::action()
{ 
  if (seen)
    {
      switch (kind)
        {
        case BAD_BSOD:
          action_bsod();
          break;
    
        case BAD_LAPTOP:
          action_bsod();
          break;
      
        case BAD_MONEY:
          action_money();
          break;
        }
    }

  /* Handle mode timer: */
  if (mode == FLAT && mode != HELD)
    {
      if(!timer_check(&timer))
        {
          mode = NORMAL;
          base.xm = 4;
        }
    }
  else if (mode == KICK)
    {
      timer_check(&timer);
    }

  // Handle dying timer:
  if (dying == DYING_SQUISHED)
    {
      /* Remove it if time's up: */
      if(!timer_check(&timer))
        bad_guys.erase(static_cast<std::vector<BadGuy>::iterator>(this));
    }

  // Remove if it's far off the screen:
  if (base.x < scroll_x - OFFSCREEN_DISTANCE)
    {
      bad_guys.erase(static_cast<std::vector<BadGuy>::iterator>(this));
      return;
    }
  else /* !seen */
    {
      // Once it's on screen, it's activated!
      if (base.x <= scroll_x + screen->w + OFFSCREEN_DISTANCE)
        seen = true;
    }
}

void
BadGuy::draw_bsod()
{
  /* --- BLUE SCREEN OF DEATH MONSTER: --- */
  if (dying == DYING_NOT)
    {
      /* Alive: */
      if (dir == LEFT)
        {
          texture_draw(&img_bsod_left[(global_frame_counter / 5) % 4],
                       base.x - scroll_x,
                       base.y);
        }
      else
        {
          texture_draw(&img_bsod_right[(global_frame_counter / 5) % 4],
                       base.x - scroll_x,
                       base.y);
        }
    }
  else if (dying == DYING_FALLING)
    {
      /* Falling: */

      if (dir == LEFT)
        {
          texture_draw(&img_bsod_falling_left,
                       base.x - scroll_x,
                       base.y);
        }
      else
        {
          texture_draw(&img_bsod_falling_right,
                       base.x - scroll_x,
                       base.y);
        }
    }
  else if (dying == DYING_SQUISHED)
    {
      /* Dying - Squished: */

      if (dir == LEFT)
        {
          texture_draw(&img_bsod_squished_left,
                       base.x - scroll_x,
                       base.y + 24);
        }
      else
        {
          texture_draw(&img_bsod_squished_right,
                       base.x - scroll_x,
                       base.y + 24);
        }
    }
}

void BadGuy::draw_laptop()
{
  /* --- LAPTOP MONSTER: --- */
  if (dying == DYING_NOT)
    {
      /* Alive: */

      if (mode == NORMAL)
        {
          /* Not flat: */
          if (dir == LEFT)
            {
              texture_draw(&img_laptop_left[(global_frame_counter / 5) % 3],
                           base.x - scroll_x,
                           base.y);
            }
          else
            {
              texture_draw(&img_laptop_right[(global_frame_counter / 5) % 3],
                           base.x - scroll_x,
                           base.y);
            }
        }
      else
        {
          /* Flat: */

          if (dir == LEFT)
            {
              texture_draw(&img_laptop_flat_left,
                           base.x - scroll_x,
                           base.y);
            }
          else
            {
              texture_draw(&img_laptop_flat_right,
                           base.x - scroll_x,
                           base.y);
            }
        }
    }
  else if (dying == DYING_FALLING)
    {
      /* Falling: */

      if (dir == LEFT)
        {
          texture_draw(&img_laptop_falling_left,
                       base.x - scroll_x,
                       base.y);
        }
      else
        {
          texture_draw(&img_laptop_falling_right,
                       base.x - scroll_x,
                       base.y);
        }
    }
}

void BadGuy::draw_money()
{
  if (base.ym != 300 /* > -16*/)
    {
      if (dir == LEFT)
        {
          texture_draw(&img_money_left[0],
                       base.x - scroll_x,
                       base.y);
        }
      else
        {
          texture_draw(&img_money_right[0],
                       base.x - scroll_x,
                       base.y);
        }
    }
  else
    {
      if (dir == LEFT)
        {
          texture_draw(&img_money_left[1],
                       base.x - scroll_x,
                       base.y);
        }
      else
        {
          texture_draw(&img_money_right[1],
                       base.x - scroll_x,
                       base.y);
        }
    }
}

void BadGuy::draw()
{
  // Don't try to draw stuff that is outside of the screen
  if (base.x > scroll_x - 32 &&
      base.x < scroll_x + screen->w)
    {
      switch (kind)
        {
        case BAD_BSOD:
          draw_bsod();
          break;
    
        case BAD_LAPTOP:
          draw_laptop();
          break;
    
        case BAD_MONEY:
          draw_money();
          break;

        default:
          puts("Unknown badguy type");
          break;
        }
    }
}

void
BadGuy::collision(void *p_c_object, int c_object)
{
  BadGuy* pbad_c    = NULL;
  Player* pplayer_c = NULL;

  switch (c_object)
    {
    case CO_BULLET:
      dying = DYING_FALLING;
      base.ym = -8;

      /* Gain some points: */
      if (kind == BAD_BSOD)
        add_score(base.x - scroll_x, base.y,
                  50 * score_multiplier);
      else if (kind == BAD_LAPTOP)
        add_score(base.x - scroll_x, base.y,
                  25 * score_multiplier);
      else if (kind == BAD_MONEY)
        add_score(base.x - scroll_x, base.y,
                  50 * score_multiplier);

      /* Play death sound: */
      play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
      break;

    case CO_BADGUY:
      pbad_c = (BadGuy*) p_c_object;
      if (mode == NORMAL)
      {
      /* do nothing */
      }
      else if(mode == KICK)
        {
          /* We're in kick mode, kill the other guy
	     and yourself(wuahaha) : */

          pbad_c->dying = DYING_FALLING;
          pbad_c->base.ym = -8;
          play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);

          add_score(base.x - scroll_x,
                    base.y, 100);
	          pbad_c->dying = DYING_FALLING;
		  
          dying = DYING_FALLING;
          base.ym = -8;

          add_score(pbad_c->base.x - scroll_x,
                    pbad_c->base.y, 100);
        }
      break;

    case CO_PLAYER:
      pplayer_c = static_cast<Player*>(p_c_object);
      if(kind != BAD_MONEY)
        {
          if (kind == BAD_BSOD)
            {
              dying = DYING_SQUISHED;
              timer_start(&timer,4000);
              physic_set_state(&pplayer_c->vphysic,PH_VT);
              physic_set_start_vy(&pplayer_c->vphysic,2.);
	      pplayer_c->base.y = base.y - pplayer_c->base.height - 1;

              add_score(base.x - scroll_x, base.y,
                        50 * score_multiplier);

              play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
            }
          else if (kind == BAD_LAPTOP)
            {

              if (mode == NORMAL || mode == KICK)
                {
                  /* Flatten! */

                  play_sound(sounds[SND_STOMP], SOUND_CENTER_SPEAKER);
                  mode = FLAT;
                  base.xm = 4;

                  timer_start(&timer,10000);

                  physic_set_state(&pplayer_c->vphysic,PH_VT);
                  physic_set_start_vy(&pplayer_c->vphysic,2.);
		  pplayer_c->base.y = base.y - pplayer_c->base.height - 1;
                }
              else if (mode == FLAT)
                {
                  /* Kick! */
                  play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

                  if (pplayer_c->base.x < base.x + (base.width/2))
                    dir = RIGHT;
                  else
                    dir = LEFT;

                  base.xm = 5;
		  mode = KICK;

                  timer_start(&timer,5000);
                }
		
              physic_set_state(&pplayer_c->vphysic,PH_VT);
              physic_set_start_vy(&pplayer_c->vphysic,2.);
	      pplayer_c->base.y = base.y - pplayer_c->base.height - 1;
	      
              add_score(base.x - scroll_x,
                        base.y,
                        25 * score_multiplier);

              /* play_sound(sounds[SND_SQUISH]); */
            }
          score_multiplier++;
        }
      break;
    }

}

// EOF //
