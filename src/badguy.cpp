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
#include <math.h>

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
texture_type img_mrbomb_left[4];
texture_type img_mrbomb_right[4];
texture_type img_mrbomb_ticking_left;
texture_type img_mrbomb_ticking_right;
texture_type img_mrbomb_explosion;
texture_type img_stalactite;
texture_type img_stalactite_broken;
texture_type img_flame[2];

BadGuyKind  badguykind_from_string(const std::string& str)
{
  if (str == "money")
    return BAD_MONEY;
  else if (str == "laptop")
    return BAD_LAPTOP;
  else if (str == "bsod")
    return BAD_BSOD;
  else if (str == "mrbomb")
    return BAD_MRBOMB;
  else if (str == "stalactite")
    return BAD_STALACTITE;
  else if (str == "flame")
    return BAD_FLAME;
  else
    {
      printf("Couldn't convert badguy: %s\n", str.c_str());
      return BAD_BSOD;
    }
}

std::string badguykind_to_string(BadGuyKind kind)
{
  switch(kind)
    {
    case BAD_MONEY:
      return "money";
      break;
    case BAD_LAPTOP:
      return "laptop";
      break;
    case BAD_BSOD:
      return "bsod";
      break;
    case BAD_MRBOMB:
      return "mrbomb";
      break;
    case BAD_STALACTITE:
      return "stalactite";
      break;
    case BAD_FLAME:
      return "flame";
      break;
    default:
      return "bsod";
    }
}

void
BadGuy::init(float x, float y, BadGuyKind kind_)
{
  base.x   = x;
  base.y   = y;    
  base.width  = 32;
  base.height = 32;
  mode     = NORMAL;
  dying    = DYING_NOT;
  kind     = kind_;
  base.xm  = -1.3;
  base.ym  = 0;
  old_base = base;
  dir      = LEFT;
  seen     = false;
  timer_init(&timer, true);
  physic_init(&physic);

  if(kind == BAD_BOMB) {
    timer_start(&timer, 1000);
    mode = BOMB_TICKING;
    // hack so that the bomb doesn't hurt until it expldes...
    dying = DYING_SQUISHED;
  } else if(kind == BAD_FLAME) {
    base.ym = 0; // we misuse base.ym as angle for the flame
  } else if(kind == BAD_MONEY) {
    base.ym = 4.8;
  }
}

void BadGuy::action_bsod()
{
  /* --- BLUE SCREEN OF DEATH MONSTER: --- */

  /* Move left/right: */
  if (dying == DYING_NOT || dying == DYING_FALLING)
    {
      base.x += base.xm * frame_ratio;
    }

  /* Move vertically: */
  base.y += base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);
		
  if (!dying)
    check_horizontal_bump();

  fall(true);

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer_check(&timer))       
    {
      /* Remove it if time's up: */
      remove_me();
      return;
    }
}

void BadGuy::action_laptop()
{
  /* Move left/right: */
  if (mode == NORMAL || mode == KICK)
    {
      if (dying == DYING_NOT ||
          dying == DYING_FALLING)
        {
          base.x += base.xm * frame_ratio;
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

      if(tux.input.fire != DOWN) /* SHOOT! */
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
  /* Bump into things horizontally: */

  if (!dying)
    {
      int changed = dir;
      check_horizontal_bump();
      if(mode == KICK && changed != dir)
        {
          /* handle stereo sound (number 10 should be tweaked...)*/
          if (base.x < scroll_x + screen->w/2 - 10)
            play_sound(sounds[SND_RICOCHET], SOUND_LEFT_SPEAKER);
          else if (base.x > scroll_x + screen->w/2 + 10)
            play_sound(sounds[SND_RICOCHET], SOUND_RIGHT_SPEAKER);
          else
            play_sound(sounds[SND_RICOCHET], SOUND_CENTER_SPEAKER);
        }
    }

  fall();

  /* Handle mode timer: */
  if (mode == FLAT)
    {
      if(!timer_check(&timer))
        {
          mode = NORMAL;
          base.xm = (dir == LEFT) ? -1.3 : 1.3;
        }
    }
}

void BadGuy::check_horizontal_bump(bool checkcliff)
{
    if (dir == LEFT && issolid( base.x, (int) base.y + 16))
    {
        dir = RIGHT;
        base.xm = -base.xm;
        return;
    }
    if (dir == RIGHT && issolid( base.x + base.width, (int) base.y + 16))
    {
        dir = LEFT;
        base.xm = -base.xm;
        return;
    }

    // don't check for cliffs when we're falling
    if(!checkcliff)
        return;
    if(!issolid(base.x + base.width/2, base.y + base.height + 16))
        return;
    
    if(dir == LEFT && !issolid(base.x, (int) base.y + base.height + 16))
    {
        printf("Cliffcol left\n");
        dir = RIGHT;
        base.xm = -base.xm;
        return;
    }
    if(dir == RIGHT && !issolid(base.x + base.width,
                (int) base.y + base.height + 16))
    {
        printf("Cliffcol right\n");
        dir = LEFT;
        base.xm = -base.xm;
        return;
    }
}

void BadGuy::fall(bool dojump)
{
  /* Fall if we get off the ground: */
  if (dying != DYING_FALLING)
    {
      if (!issolid(base.x+base.width/2, base.y + base.height))
        {
          if(!physic_is_set(&physic))
            {
              physic_set_state(&physic,PH_VT);
              physic_set_start_vy(&physic, dojump ? 2. : 0.);
            }

          base.ym = physic_get_velocity(&physic);
        }
      else
        {
          /* Land: */
          if (base.ym > 0)
            {
              base.y = int((base.y + base.height)/32) * 32 - base.height;
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

  // BadGuy fall below the ground
  if (base.y > screen->h) {
    remove_me();
    return;
  }
}

void BadGuy::remove_me()
{
  std::vector<BadGuy>::iterator i;
  for(i = bad_guys.begin(); i != bad_guys.end(); ++i) {
    if( & (*i) == this) {
      bad_guys.erase(i);
      return;
    }
  }
}

void BadGuy::action_money()
{
  /* Move vertically: */
  base.y = base.y + base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);

  if (base.y > screen->h) {
    remove_me();
    return;
  }

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
          mode = MONEY_JUMP;
        }
      else
        {
          base.ym = physic_get_velocity(&physic);
          mode = NORMAL;
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

void BadGuy::action_mrbomb()
{
  if(mode == NORMAL) {
    base.x += base.xm * frame_ratio;
  }

  /* Move vertically: */
  base.y += base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);

  check_horizontal_bump(true);
  fall();
}

void BadGuy::action_bomb()
{
  // eventually fall down
  base.y += base.ym * frame_ratio;
  collision_swept_object_map(&old_base,&base);
  fall();

  if(!timer_check(&timer)) {
    if(mode == BOMB_TICKING) {
      mode = BOMB_EXPLODE;
      dying = DYING_NOT; // now the bomb hurts
      timer_start(&timer, 1000);
      // explosion image has different size
      base.x -= (img_mrbomb_explosion.w - base.width) / 2;
      base.y -= img_mrbomb_explosion.h - base.height;
      base.width = img_mrbomb_explosion.w;
      base.height = img_mrbomb_explosion.h;
      old_base = base;
    } else if(mode == BOMB_EXPLODE) {
      remove_me();
      return;
    }
  }
}

void BadGuy::action_stalactite()
{
  if(mode == NORMAL) {
    static const int range = 40;
    // start shaking when tux is below the stalactite and at least 40 pixels
    // near
    if(tux.base.x + 32 > base.x - range && tux.base.x < base.x + 32 + range
            && tux.base.y + tux.base.height > base.y) {
      timer_start(&timer, 800);
      mode = STALACTITE_SHAKING;
    }
  } if(mode == STALACTITE_SHAKING) {
    base.x = old_base.x + (rand() % 6) - 3; // TODO this could be done nicer...
    if(!timer_check(&timer)) {
      mode = STALACTITE_FALL;
    }
  } else if(mode == STALACTITE_FALL) {
    base.y += base.ym * frame_ratio;
    fall();
    /* Destroy if collides land */
    if(issolid(base.x+16, base.y+32))
    {
      timer_start(&timer, 3000);
      dying = DYING_SQUISHED;
      mode = FLAT;
    }
  } else if(mode == FLAT) {
    if(!timer_check(&timer)) {
      remove_me();
    }
  }
}

void
BadGuy::action_flame()
{
    static const float radius = 100;
    static const float speed = 0.02;
    base.x = old_base.x + cos(base.ym) * radius;
    base.y = old_base.y + sin(base.ym) * radius;

    base.ym = fmodf(base.ym + frame_ratio * speed, 2*M_PI);
}

void
BadGuy::action()
{
  // Remove if it's far off the screen:
  if (base.x < scroll_x - OFFSCREEN_DISTANCE)
    {
      remove_me();                                                
      return;
    }

  // Once it's on screen, it's activated!
  if (base.x <= scroll_x + screen->w + OFFSCREEN_DISTANCE)
    seen = true;

  if(!seen)
    return;

  switch (kind)
    {
    case BAD_BSOD:
      action_bsod();
      break;

    case BAD_LAPTOP:
      action_laptop();
      break;
  
    case BAD_MONEY:
      action_money();
      break;

    case BAD_MRBOMB:
      action_mrbomb();
      break;
    
    case BAD_BOMB:
      action_bomb();
      break;

    case BAD_STALACTITE:
      action_stalactite();
      break;

    case BAD_FLAME:
      action_flame();
      break;
    }
}

void
BadGuy::draw_bsod()
{
  texture_type* texture = 0;
  
  if(dying == DYING_NOT) {
    size_t frame = (global_frame_counter / 5) % 4;
    texture = (dir == LEFT) ? &img_bsod_left[frame] : &img_bsod_right[frame];
  } else if(dying == DYING_FALLING) {
    texture = (dir == LEFT) ? &img_bsod_falling_left : &img_bsod_falling_right;
  } else if(dying == DYING_SQUISHED) {
    texture = (dir == LEFT) 
        ? &img_bsod_squished_left : &img_bsod_squished_right;
  }
  
  texture_draw(texture, base.x - scroll_x, base.y);
}

void
BadGuy::draw_laptop()
{
  texture_type* texture;
  size_t frame = (global_frame_counter / 5) % 3;
  
  if(dying == DYING_NOT) {
    if(mode == NORMAL) {
      if(dir == LEFT)
        texture = &img_laptop_left[frame];
      else
        texture = &img_laptop_right[frame];
    } else {
      texture = (dir == LEFT) ? &img_laptop_flat_left : &img_laptop_flat_right;
    }
  } else {
    texture = (dir == LEFT) 
        ? &img_laptop_falling_left : &img_laptop_falling_right;
  }

  texture_draw(texture, base.x - scroll_x, base.y);
}

void
BadGuy::draw_money()
{
  texture_type* texture;
  size_t frame = (mode == NORMAL) ? 0 : 1;

  if(tux.base.x + tux.base.width < base.x) {
    texture = &img_money_left[frame];
  } else {
    texture = &img_money_right[frame];
  }

  texture_draw(texture, base.x - scroll_x, base.y);
}
  
void
BadGuy::draw_mrbomb()
{
  texture_type* texture;
  size_t frame = (global_frame_counter/5) % 4;

  if(dir == LEFT)
    texture = &img_mrbomb_left[frame];
  else
    texture = &img_mrbomb_right[frame];

  texture_draw(texture, base.x - scroll_x, base.y);
}

void
BadGuy::draw_bomb()
{
  texture_type* texture;

  // TODO add real bomb graphics
  if(mode == BOMB_TICKING) {
    texture = (dir == LEFT) 
        ? &img_mrbomb_ticking_left : &img_mrbomb_ticking_right;
  } else {
    texture = &img_mrbomb_explosion;
  }
  
  texture_draw(texture, base.x - scroll_x, base.y);
}

void
BadGuy::draw_stalactite()
{
  texture_type* texture;
  if(mode != FLAT)
    texture = &img_stalactite;
  else
    texture = &img_stalactite_broken;

  texture_draw(texture, base.x - scroll_x, base.y);
}

void
BadGuy::draw_flame()
{
  size_t frame = (global_frame_counter / 10) % 2;
  texture_type* texture = &img_flame[frame];

  texture_draw(texture, base.x - scroll_x, base.y);
}

void
BadGuy::draw()
{
  // Don't try to draw stuff that is outside of the screen
  if (base.x <= scroll_x - base.width || base.x >= scroll_x + screen->w)
    return;
      
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

    case BAD_MRBOMB:
      draw_mrbomb();
      break;

    case BAD_BOMB:
      draw_bomb();
      break;

    case BAD_STALACTITE:
      draw_stalactite();
      break;

    case BAD_FLAME:
      draw_flame();
      break;

    }
}

void
BadGuy::bump()
{
  if(kind == BAD_BSOD || kind == BAD_LAPTOP || kind == BAD_BOMB) {
    dying = DYING_FALLING;
    base.ym = -8;
    play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
  }
}

void
BadGuy::make_player_jump(Player* player)
{
    physic_set_state(&player->vphysic,PH_VT);
    physic_set_start_vy(&player->vphysic, 2.);
    player->base.y = base.y - player->base.height - 2;
}

void
BadGuy::squich(Player* player)
{
  if(kind == BAD_MRBOMB) {
      // mrbomb transforms into a bomb now
      add_bad_guy(base.x, base.y, BAD_BOMB);
      
      make_player_jump(player);
      add_score(base.x - scroll_x, base.y, 50 * score_multiplier);
      play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
      score_multiplier++;
      
      remove_me();
      return;

  } else if(kind == BAD_BSOD) {
      make_player_jump(player);

      add_score(base.x - scroll_x, base.y, 50 * score_multiplier);
      play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
      score_multiplier++;

      dying = DYING_SQUISHED;
      timer_start(&timer, 2000);
      base.y += base.height - img_bsod_squished_left.h;
      base.height = img_bsod_squished_left.h;
      base.xm = base.ym = 0;
      old_base = base;
      return;
      
  } else if (kind == BAD_LAPTOP) {
      if (mode == NORMAL || mode == KICK)
      {
          /* Flatten! */
          play_sound(sounds[SND_STOMP], SOUND_CENTER_SPEAKER);
          mode = FLAT;
          base.xm = 0;

          timer_start(&timer, 4000);
      } else if (mode == FLAT) {
          /* Kick! */
          play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

          if (player->base.x < base.x + (base.width/2)) {
              base.xm = 5;
              dir = RIGHT;
          } else {
              base.xm = -5;
              dir = LEFT;
          }

          mode = KICK;
      }

      make_player_jump(player);
	      
      add_score(base.x - scroll_x, base.y, 25 * score_multiplier);
      score_multiplier++;
      return;
  }
}

void
BadGuy::collision(void *p_c_object, int c_object, CollisionType type)
{
  BadGuy* pbad_c    = NULL;

  if(type == COLLISION_BUMP) {
    bump();
    return;
  }
  if(type == COLLISION_SQUICH) {
    Player* player = static_cast<Player*>(p_c_object);
    squich(player);
    return;
  }

  switch (c_object)
    {
    case CO_BULLET:
      if(kind == BAD_BOMB || kind == BAD_STALACTITE || kind == BAD_FLAME)
        return;

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
      if(kind == BAD_LAPTOP && mode == KICK)
        {
          /* We're in kick mode, kill the other guy
	     and yourself(wuahaha) : */

          pbad_c->dying = DYING_FALLING;
          pbad_c->base.ym = -8;
          play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);

          add_score(base.x - scroll_x,
                    base.y, 100);
		  
          dying = DYING_FALLING;
          base.ym = -8;

          add_score(pbad_c->base.x - scroll_x,
                    pbad_c->base.y, 100);
        }
      break;
    }
}

//---------------------------------------------------------------------------

void load_badguy_gfx()
{
  /* (BSOD) */
  texture_load(&img_bsod_left[0], datadir +
               "/images/shared/bsod-left-0.png",
               USE_ALPHA);

  texture_load(&img_bsod_left[1], datadir +
               "/images/shared/bsod-left-1.png",
               USE_ALPHA);

  texture_load(&img_bsod_left[2], datadir +
               "/images/shared/bsod-left-2.png",
               USE_ALPHA);

  texture_load(&img_bsod_left[3], datadir +
               "/images/shared/bsod-left-3.png",
               USE_ALPHA);

  texture_load(&img_bsod_right[0], datadir +
               "/images/shared/bsod-right-0.png",
               USE_ALPHA);

  texture_load(&img_bsod_right[1], datadir +
               "/images/shared/bsod-right-1.png",
               USE_ALPHA);

  texture_load(&img_bsod_right[2], datadir +
               "/images/shared/bsod-right-2.png",
               USE_ALPHA);

  texture_load(&img_bsod_right[3], datadir +
               "/images/shared/bsod-right-3.png",
               USE_ALPHA);

  texture_load(&img_bsod_squished_left, datadir +
               "/images/shared/bsod-squished-left.png",
               USE_ALPHA);

  texture_load(&img_bsod_squished_right, datadir +
               "/images/shared/bsod-squished-right.png",
               USE_ALPHA);

  texture_load(&img_bsod_falling_left, datadir +
               "/images/shared/bsod-falling-left.png",
               USE_ALPHA);

  texture_load(&img_bsod_falling_right, datadir +
               "/images/shared/bsod-falling-right.png",
               USE_ALPHA);


  /* (Laptop) */

  texture_load(&img_laptop_left[0], datadir +
               "/images/shared/laptop-left-0.png",
               USE_ALPHA);

  texture_load(&img_laptop_left[1], datadir +
               "/images/shared/laptop-left-1.png",
               USE_ALPHA);

  texture_load(&img_laptop_left[2], datadir +
               "/images/shared/laptop-left-2.png",
               USE_ALPHA);

  texture_load(&img_laptop_right[0], datadir +
               "/images/shared/laptop-right-0.png",
               USE_ALPHA);

  texture_load(&img_laptop_right[1], datadir +
               "/images/shared/laptop-right-1.png",
               USE_ALPHA);

  texture_load(&img_laptop_right[2], datadir +
               "/images/shared/laptop-right-2.png",
               USE_ALPHA);

  texture_load(&img_laptop_flat_left, datadir +
               "/images/shared/laptop-flat-left.png",
               USE_ALPHA);

  texture_load(&img_laptop_flat_right, datadir +
               "/images/shared/laptop-flat-right.png",
               USE_ALPHA);

  texture_load(&img_laptop_falling_left, datadir +
               "/images/shared/laptop-falling-left.png",
               USE_ALPHA);

  texture_load(&img_laptop_falling_right, datadir +
               "/images/shared/laptop-falling-right.png",
               USE_ALPHA);


  /* (Money) */

  texture_load(&img_money_left[0], datadir +
               "/images/shared/bag-left-0.png",
               USE_ALPHA);

  texture_load(&img_money_left[1], datadir +
               "/images/shared/bag-left-1.png",
               USE_ALPHA);

  texture_load(&img_money_right[0], datadir +
               "/images/shared/bag-right-0.png",
               USE_ALPHA);

  texture_load(&img_money_right[1], datadir +
               "/images/shared/bag-right-1.png",
               USE_ALPHA);

  /* Mr. Bomb */
  for(int i=0; i<4; ++i) {
      char num[4];
      snprintf(num, 4, "%d", i);
      texture_load(&img_mrbomb_left[i],
              datadir + "/images/shared/mrbomb-left-" + num + ".png", USE_ALPHA);
      texture_load(&img_mrbomb_right[i],
              datadir + "/images/shared/mrbomb-right-" + num + ".png", USE_ALPHA);
  }
  texture_load(&img_mrbomb_ticking_left,
          datadir + "/images/shared/mrbombx-left-0.png", USE_ALPHA);
  texture_load(&img_mrbomb_ticking_right,
          datadir + "/images/shared/mrbombx-right-0.png", USE_ALPHA);
  texture_load(&img_mrbomb_explosion,
          datadir + "/images/shared/mrbomb-explosion.png", USE_ALPHA);

  /* stalactite */
  texture_load(&img_stalactite, 
          datadir + "/images/shared/stalactite.png", USE_ALPHA);
  texture_load(&img_stalactite_broken,
          datadir + "/images/shared/stalactite-broken.png", USE_ALPHA);

  /* flame */
  texture_load(&img_flame[0],
          datadir + "/images/shared/flame-0.png", USE_ALPHA);
  texture_load(&img_flame[1],
          datadir + "/images/shared/flame-1.png", USE_ALPHA);  
}

void free_badguy_gfx()
{
  for (int i = 0; i < 4; i++)
    {
      texture_free(&img_bsod_left[i]);
      texture_free(&img_bsod_right[i]);
    }

  texture_free(&img_bsod_squished_left);
  texture_free(&img_bsod_squished_right);

  texture_free(&img_bsod_falling_left);
  texture_free(&img_bsod_falling_right);

  for (int i = 0; i < 3; i++)
    {
      texture_free(&img_laptop_left[i]);
      texture_free(&img_laptop_right[i]);
    }

  texture_free(&img_laptop_flat_left);
  texture_free(&img_laptop_flat_right);

  texture_free(&img_laptop_falling_left);
  texture_free(&img_laptop_falling_right);

  for (int i = 0; i < 2; i++)
    {
      texture_free(&img_money_left[i]);
      texture_free(&img_money_right[i]);
    }

  for(int i = 0; i < 4; i++) {
      texture_free(&img_mrbomb_left[i]);
      texture_free(&img_mrbomb_right[i]);
  }

  texture_free(&img_mrbomb_ticking_left);
  texture_free(&img_mrbomb_ticking_right);
  texture_free(&img_mrbomb_explosion);

  texture_free(&img_stalactite);
  texture_free(&img_stalactite_broken);

  texture_free(&img_flame[0]);
  texture_free(&img_flame[1]);
}

// EOF //
