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
texture_type img_mrbomb_left[4];
texture_type img_mrbomb_right[4];
texture_type img_stalactite;
texture_type img_stalactite_broken;

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
    default:
      return "bsod";
    }
}

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
  base.xm  = -1.3;
  base.ym  = 4.8;
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
  }
}

void BadGuy::action_bsod()
{
  /* --- BLUE SCREEN OF DEATH MONSTER: --- */

  /* Move left/right: */
  if (dying == DYING_NOT ||
      dying == DYING_FALLING)
    {
      base.x += base.xm * frame_ratio;
    }

  /* Move vertically: */
  base.y = base.y + base.ym * frame_ratio;

  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);
		
  if (!dying)
    check_horizontal_bump();

  fall();

  // Handle dying timer:
  if (dying == DYING_SQUISHED)       
    {
      /* Remove it if time's up: */
      if(!timer_check(&timer)) {
        remove_me();
        return;
      }
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
          if (base.x < scroll_x - 10)
            play_sound(sounds[SND_RICOCHET], SOUND_LEFT_SPEAKER);
          else if (base.x > scroll_x + 10)
            play_sound(sounds[SND_RICOCHET], SOUND_RIGHT_SPEAKER);
          else
            play_sound(sounds[SND_RICOCHET], SOUND_CENTER_SPEAKER);
        }
    }

  fall();

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

void BadGuy::fall()
{
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
        }
      /* // matze: is this code needed?
      else if(issolid(base.x, base.y))
        { // This works, but isn't the best solution imagineable 
          physic_set_state(&physic,PH_VT);
          physic_set_start_vy(&physic,0.);
          base.ym = physic_get_velocity(&physic);
          ++base.y;
        }*/
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
    } else if(mode == BOMB_EXPLODE) {
      remove_me();
      return;
    }
  }
}

void BadGuy::action_stalactite()
{
  if(mode == NORMAL) {
    if(tux.base.x + 32 > base.x - 40 && tux.base.x < base.x + 32 + 40) {
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
    /* Destroy if collides land */
    if(issolid(base.x+16, base.y+32))
    {
      timer_start(&timer, 3000);
      dying = DYING_SQUISHED;
      mode = FLAT;
    }
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

        }
    }

  // Remove if it's far off the screen:
  if (base.x < scroll_x - OFFSCREEN_DISTANCE)
    {
      remove_me();
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
  texture_type* texture = 0;
  float y = base.y;
  if(dying == DYING_NOT) {
    size_t frame = (global_frame_counter / 5) % 4;
    texture = (dir == LEFT) ? &img_bsod_left[frame] : &img_bsod_right[frame];
  } else if(dying == DYING_FALLING) {
    texture = (dir == LEFT) ? &img_bsod_falling_left : &img_bsod_falling_right;
  } else if(dying == DYING_SQUISHED) {
    texture = (dir == LEFT) 
        ? &img_bsod_squished_left : &img_bsod_squished_right;
    y += 24;
  }
  
  texture_draw(texture, base.x - scroll_x, y);
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
  size_t frame = (base.ym != 300) ? 0 : 1;

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
    texture = &img_bsod_squished_right;
  } else {
    texture = &img_bsod_squished_left;
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
BadGuy::draw()
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

        case BAD_MRBOMB:
          draw_mrbomb();
          break;

        case BAD_BOMB:
          draw_bomb();
          break;

        case BAD_STALACTITE:
          draw_stalactite();
          break;
        }
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
    physic_set_start_vy(&player->vphysic,2.);
    player->base.y = base.y - player->base.height - 1;
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
      dying = DYING_SQUISHED;
      timer_start(&timer,4000);

      make_player_jump(player);

      add_score(base.x - scroll_x, base.y, 50 * score_multiplier);
      play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
      score_multiplier++;
      return;
      
  } else if (kind == BAD_LAPTOP) {
      if (mode == NORMAL || mode == KICK)
      {
          /* Flatten! */
          play_sound(sounds[SND_STOMP], SOUND_CENTER_SPEAKER);
          mode = FLAT;
          base.xm = 4;

          timer_start(&timer, 4000);
      } else if (mode == FLAT) {
          /* Kick! */
          play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

          if (player->base.x < base.x + (base.width/2))
              dir = RIGHT;
          else
              dir = LEFT;

          base.xm = 5;
          mode = KICK;

          timer_start(&timer,5000);
      }

      make_player_jump(player);
	      
      add_score(base.x - scroll_x,
              base.y,
              25 * score_multiplier);
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
      if(kind == BAD_BOMB || kind == BAD_STALACTITE)
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
    }
}

// EOF //
