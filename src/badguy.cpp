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
#include "world.h"
#include "tile.h"

texture_type img_bsod_squished_left[1];
texture_type img_bsod_squished_right[1];
texture_type img_bsod_falling_left[1];
texture_type img_bsod_falling_right[1];
texture_type img_laptop_flat_left[1];
texture_type img_laptop_flat_right[1];
texture_type img_laptop_falling_left[1];
texture_type img_laptop_falling_right[1];
texture_type img_bsod_left[4];
texture_type img_bsod_right[4];
texture_type img_laptop_left[4];
texture_type img_laptop_right[4];
texture_type img_money_left[2];
texture_type img_money_right[2];
texture_type img_mrbomb_left[4];
texture_type img_mrbomb_right[4];
texture_type img_mrbomb_ticking_left[1];
texture_type img_mrbomb_ticking_right[1];
texture_type img_mrbomb_explosion[1];
texture_type img_stalactite[1];
texture_type img_stalactite_broken[1];
texture_type img_flame[2];
texture_type img_fish[2];
texture_type img_bouncingsnowball_left[6];
texture_type img_bouncingsnowball_right[6];
texture_type img_bouncingsnowball_squished[1];
texture_type img_flyingsnowball[2];
texture_type img_flyingsnowball_squished[1];
texture_type img_spiky_left[3];
texture_type img_spiky_right[3];
texture_type img_snowball_left[4];
texture_type img_snowball_right[4];
texture_type img_snowball_squished_left[1];
texture_type img_snowball_squished_right[1];

BadGuyKind  badguykind_from_string(const std::string& str)
{
  if (str == "money")
    return BAD_MONEY;
  else if (str == "laptop" || str == "mriceblock")
    return BAD_LAPTOP;
  else if (str == "bsod")
    return BAD_BSOD;
  else if (str == "mrbomb")
    return BAD_MRBOMB;
  else if (str == "stalactite")
    return BAD_STALACTITE;
  else if (str == "flame")
    return BAD_FLAME;
  else if (str == "fish")
    return BAD_FISH;
  else if (str == "bouncingsnowball")
    return BAD_BOUNCINGSNOWBALL;
  else if (str == "flyingsnowball")
    return BAD_FLYINGSNOWBALL;
  else if (str == "spiky")
    return BAD_SPIKY;
  else if (str == "snowball")
    return BAD_SNOWBALL;
  else
    {
      printf("Couldn't convert badguy: '%s'\n", str.c_str());
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
    case BAD_FISH:
      return "fish";
      break;
    case BAD_BOUNCINGSNOWBALL:
      return "bouncingsnowball";
      break;
    case BAD_FLYINGSNOWBALL:
      return "flyingsnowball";
      break;
    case BAD_SPIKY:
      return "spiky";
      break;
    case BAD_SNOWBALL:
      return "snowball";
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
  base.width  = 0;
  base.height = 0;
  base.xm  = 0;
  base.ym  = 0;

  mode     = NORMAL;
  dying    = DYING_NOT;
  kind     = kind_;
  old_base = base;
  dir      = LEFT;
  seen     = false;
  animation_speed = 1;
  animation_length = 1;
  animation_offset = 0;
  texture_left = texture_right = 0;
  physic.reset();
  timer.init(true);

  if(kind == BAD_BSOD) {
    physic.set_velocity(-1.3, 0);
    set_texture(img_bsod_left, img_bsod_right, 4);
  } else if(kind == BAD_MRBOMB) {
    physic.set_velocity(-1.3, 0);
    set_texture(img_mrbomb_left, img_mrbomb_right, 4);
  } else if (kind == BAD_LAPTOP) {
    physic.set_velocity(-1.3, 0);
    set_texture(img_laptop_left, img_laptop_right, 4, 5);
  } else if(kind == BAD_MONEY) {
    set_texture(img_money_left, img_money_right, 1);
  } else if(kind == BAD_BOMB) {
    set_texture(img_mrbomb_ticking_left, img_mrbomb_ticking_right, 1);
    // hack so that the bomb doesn't hurt until it expldes...
    dying = DYING_SQUISHED;
  } else if(kind == BAD_FLAME) {
    base.ym = 0; // we misuse base.ym as angle for the flame
    physic.enable_gravity(false);
    set_texture(img_flame, img_flame, 2, 0.5);
  } else if(kind == BAD_BOUNCINGSNOWBALL) {
    physic.set_velocity(-1.3, 0);
    set_texture(img_bouncingsnowball_left, img_bouncingsnowball_right, 6);
  } else if(kind == BAD_STALACTITE) {
    physic.enable_gravity(false);
    set_texture(img_stalactite, img_stalactite, 1);
  } else if(kind == BAD_FISH) {
    set_texture(img_fish, img_fish, 2, 1);
    physic.enable_gravity(true);
  } else if(kind == BAD_FLYINGSNOWBALL) {
    set_texture(img_flyingsnowball, img_flyingsnowball, 2, 5);
    physic.enable_gravity(false);
  } else if(kind == BAD_SPIKY) {
    physic.set_velocity(-1.3, 0);
    set_texture(img_spiky_left, img_spiky_right, 3);
  } else if(kind == BAD_SNOWBALL) {
    physic.set_velocity(-1.3, 0);
    set_texture(img_snowball_left, img_snowball_right, 4, 5);
  }

  // if we're in a solid tile at start correct that now
  if(kind != BAD_FLAME && kind != BAD_FISH && collision_object_map(&base)) {
    printf("Warning: badguy started in wall!.\n");
    while(collision_object_map(&base))
      --base.y;
  }
}

void
BadGuy::action_bsod(float frame_ratio)
{
  static const float BSODJUMP = 2;
    
  if (dying == DYING_NOT)
    check_horizontal_bump();

  fall();

  // jump when we're about to fall
  if (physic.get_velocity_y() == 0 && 
          !issolid(base.x+base.width/2, base.y + base.height)) {
    physic.enable_gravity(true);
    physic.set_velocity(physic.get_velocity_x(), BSODJUMP);
  }

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    {
      /* Remove it if time's up: */
      remove_me();
      return;
    }

  // move
  physic.apply(frame_ratio, base.x, base.y);
  if(dying != DYING_FALLING)
    collision_swept_object_map(&old_base, &base);
}

void
BadGuy::action_laptop(float frame_ratio)
{
  Player& tux = *World::current()->get_tux();

  if(dying == DYING_NOT)
    fall();
  
  /* Move left/right: */
  if (mode == NORMAL || mode == KICK)
    {
      // move
      physic.apply(frame_ratio, base.x, base.y);
      if (dying != DYING_FALLING)
        collision_swept_object_map(&old_base,&base);
    }
  else if (mode == HELD)
    { /* FIXME: The pbad object shouldn't know about pplayer objects. */
      /* If we're holding the laptop */
      dir = tux.dir;
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
          old_base = base;

          mode=KICK;
          set_texture(img_laptop_flat_left, img_laptop_flat_right, 1);
          physic.set_velocity((dir == LEFT) ? -8 : 8, -8);
          play_sound(sounds[SND_KICK],SOUND_CENTER_SPEAKER);
        }
    }

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

  /* Handle mode timer: */
  if (mode == FLAT)
    {
      if(!timer.check())
        {
          mode = NORMAL;
          set_texture(img_laptop_left, img_laptop_right, 4, 5);
          physic.set_velocity( (dir == LEFT) ? -1.3 : 1.3, 0);
        }
    }
}

void
BadGuy::check_horizontal_bump(bool checkcliff)
{
    float halfheight = base.height / 2;
    if (dir == LEFT && issolid( base.x, (int) base.y + halfheight))
    {
        dir = RIGHT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }
    if (dir == RIGHT && issolid( base.x + base.width, (int)base.y + halfheight))
    {
        dir = LEFT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }

    // don't check for cliffs when we're falling
    if(!checkcliff)
        return;
    if(!issolid(base.x + base.width/2, base.y + base.height))
        return;
    
    if(dir == LEFT && !issolid(base.x, (int) base.y + base.height + halfheight))
    {
        dir = RIGHT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }
    if(dir == RIGHT && !issolid(base.x + base.width,
                (int) base.y + base.height + halfheight))
    {
        dir = LEFT;
        physic.set_velocity(-physic.get_velocity_x(), physic.get_velocity_y());
        return;
    }
}

void
BadGuy::fall()
{
  /* Fall if we get off the ground: */
  if (dying != DYING_FALLING)
    {
      if (!issolid(base.x+base.width/2, base.y + base.height))
        {
          // not solid below us? enable gravity
          physic.enable_gravity(true);
        }
      else
        {
          /* Land: */
          if (physic.get_velocity_y() < 0)
            {
              base.y = int((base.y + base.height)/32) * 32 - base.height;
              physic.set_velocity(physic.get_velocity_x(), 0);
            }
          // no gravity anymore please
          physic.enable_gravity(false);
        }
    }
  else
    {
      physic.enable_gravity(true);
    }
}

void
BadGuy::remove_me()
{
  for(std::vector<BadGuy>::iterator i = World::current()->bad_guys.begin(); 
      i != World::current()->bad_guys.end(); ++i) 
    {
      if( & (*i) == this) {
        World::current()->bad_guys.erase(i);
        return;
      }
    }
}

void
BadGuy::action_money(float frame_ratio)
{
  Player& tux = *World::current()->get_tux();

  static const float JUMPV = 6;
    
  fall();
  // jump when on ground
  if(dying == DYING_NOT && issolid(base.x, base.y+32))
    {
      physic.set_velocity(physic.get_velocity_x(), JUMPV);
      physic.enable_gravity(true);
      set_texture(&img_money_left[1], &img_money_right[1], 1);
      mode = MONEY_JUMP;
    }
  else if(mode == MONEY_JUMP)
    {
      set_texture(&img_money_left[0], &img_money_right[0], 1);
      mode = NORMAL;
    }

  // set direction based on tux
  if(tux.base.x > base.x)
    dir = RIGHT;
  else
    dir = LEFT;

  // move
  physic.apply(frame_ratio, base.x, base.y);
  if(dying == DYING_NOT)
    collision_swept_object_map(&old_base, &base);
}

void
BadGuy::action_mrbomb(float frame_ratio)
{
  if (dying == DYING_NOT)
    check_horizontal_bump(true);

  fall();

  physic.apply(frame_ratio, base.x, base.y);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base); 
}

void
BadGuy::action_bomb(float frame_ratio)
{
  static const int TICKINGTIME = 1000;
  static const int EXPLODETIME = 1000;
    
  fall();

  if(mode == NORMAL) {
    mode = BOMB_TICKING;
    timer.start(TICKINGTIME);
  } else if(!timer.check()) {
    if(mode == BOMB_TICKING) {
      mode = BOMB_EXPLODE;
      set_texture(img_mrbomb_explosion, img_mrbomb_explosion, 1);
      dying = DYING_NOT; // now the bomb hurts
      timer.start(EXPLODETIME);
    } else if(mode == BOMB_EXPLODE) {
      remove_me();
      return;
    }
  }

  // move
  physic.apply(frame_ratio, base.x, base.y);                 
  collision_swept_object_map(&old_base,&base);
}

void
BadGuy::action_stalactite(float frame_ratio)
{
  Player& tux = *World::current()->get_tux();

  static const int SHAKETIME = 800;
  static const int RANGE = 40;
    
  if(mode == NORMAL) {
    // start shaking when tux is below the stalactite and at least 40 pixels
    // near
    if(tux.base.x + 32 > base.x - RANGE && tux.base.x < base.x + 32 + RANGE
            && tux.base.y + tux.base.height > base.y) {
      timer.start(SHAKETIME);
      mode = STALACTITE_SHAKING;
    }
  } if(mode == STALACTITE_SHAKING) {
    base.x = old_base.x + (rand() % 6) - 3; // TODO this could be done nicer...
    if(!timer.check()) {
      mode = STALACTITE_FALL;
    }
  } else if(mode == STALACTITE_FALL) {
    fall();
    /* Destroy if we collides with land */
    if(issolid(base.x+base.width/2, base.y+base.height))
    {
      timer.start(2000);
      dying = DYING_SQUISHED;
      mode = FLAT;
      set_texture(img_stalactite_broken, img_stalactite_broken, 1);
    }
  } else if(mode == FLAT) {
    fall();
  }

  // move
  physic.apply(frame_ratio, base.x, base.y);

  if(dying == DYING_SQUISHED && !timer.check())
    remove_me();
}

void
BadGuy::action_flame(float frame_ratio)
{
    static const float radius = 100;
    static const float speed = 0.02;
    base.x = old_base.x + cos(base.ym) * radius;
    base.y = old_base.y + sin(base.ym) * radius;

    base.ym = fmodf(base.ym + frame_ratio * speed, 2*M_PI);
}

void
BadGuy::action_fish(float frame_ratio)
{
  static const float JUMPV = 6;
  static const int WAITTIME = 1000;
    
  // go in wait mode when back in water
  if(dying == DYING_NOT && gettile(base.x, base.y+ base.height)->water
        && physic.get_velocity_y() <= 0 && mode == NORMAL)
    {
      mode = FISH_WAIT;
      set_texture(0, 0);
      physic.set_velocity(0, 0);
      physic.enable_gravity(false);
      timer.start(WAITTIME);
    }
  else if(mode == FISH_WAIT && !timer.check())
    {
      // jump again
      set_texture(img_fish, img_fish, 2, 1.5);
      animation_offset = global_frame_counter; // restart animation
      mode = NORMAL;
      physic.set_velocity(0, JUMPV);
      physic.enable_gravity(true);
    }

  physic.apply(frame_ratio, base.x, base.y);
  if(dying == DYING_NOT)
    collision_swept_object_map(&old_base, &base);
}

void
BadGuy::action_bouncingsnowball(float frame_ratio)
{
  static const float JUMPV = 4.5;
    
  fall();

  // jump when on ground
  if(dying == DYING_NOT && issolid(base.x, base.y+32))
    {
      physic.set_velocity(physic.get_velocity_x(), JUMPV);
      physic.enable_gravity(true);
    }                                                     
  else
    {
      mode = NORMAL;
    }

  // check for right/left collisions
  check_horizontal_bump();

  physic.apply(frame_ratio, base.x, base.y);
  if(dying == DYING_NOT)
    collision_swept_object_map(&old_base, &base);

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    {
      /* Remove it if time's up: */
      remove_me();
      return;
    }
}

void
BadGuy::action_flyingsnowball(float frame_ratio)
{
  static const float FLYINGSPEED = 1;
  static const int DIRCHANGETIME = 1000;
    
  // go into flyup mode if none specified yet
  if(dying == DYING_NOT && mode == NORMAL) {
    mode = FLY_UP;
    physic.set_velocity(physic.get_velocity_x(), FLYINGSPEED);
    timer.start(DIRCHANGETIME/2);
  }

  if(dying == DYING_NOT && !timer.check()) {
    if(mode == FLY_UP) {
      mode = FLY_DOWN;
      physic.set_velocity(physic.get_velocity_x(), -FLYINGSPEED);
    } else if(mode == FLY_DOWN) {
      mode = FLY_UP;
      physic.set_velocity(physic.get_velocity_x(), FLYINGSPEED);
    }
    timer.start(DIRCHANGETIME);
  }

  if(dying != DYING_NOT)
    physic.enable_gravity(true);

  physic.apply(frame_ratio, base.x, base.y);
  if(dying == DYING_NOT || dying == DYING_SQUISHED)
    collision_swept_object_map(&old_base, &base);

  // Handle dying timer:
  if (dying == DYING_SQUISHED && !timer.check())
    {
      /* Remove it if time's up: */
      remove_me();
      return;
    }                                                          
}

void
BadGuy::action_spiky(float frame_ratio)
{
  if (dying == DYING_NOT)
    check_horizontal_bump();

  fall();
#if 0
  // jump when we're about to fall
  if (physic.get_velocity_y() == 0 && 
          !issolid(base.x+base.width/2, base.y + base.height)) {
    physic.enable_gravity(true);
    physic.set_velocity(physic.get_velocity_x(), 2);
  }
#endif

  physic.apply(frame_ratio, base.x, base.y);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);   
}

void
BadGuy::action_snowball(float frame_ratio)
{
  if (dying == DYING_NOT)
    check_horizontal_bump();

  fall();

  physic.apply(frame_ratio, base.x, base.y);
  if (dying != DYING_FALLING)
    collision_swept_object_map(&old_base,&base);
}

void
BadGuy::action(float frame_ratio)
{
  // Remove if it's far off the screen:
  if (base.x < scroll_x - OFFSCREEN_DISTANCE)
    {
      remove_me();                                                
      return;
    }

  // BadGuy fall below the ground
  if (base.y > screen->h) {
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
      action_bsod(frame_ratio);
      break;

    case BAD_LAPTOP:
      action_laptop(frame_ratio);
      break;
  
    case BAD_MONEY:
      action_money(frame_ratio);
      break;

    case BAD_MRBOMB:
      action_mrbomb(frame_ratio);
      break;
    
    case BAD_BOMB:
      action_bomb(frame_ratio);
      break;

    case BAD_STALACTITE:
      action_stalactite(frame_ratio);
      break;

    case BAD_FLAME:
      action_flame(frame_ratio);
      break;

    case BAD_FISH:
      action_fish(frame_ratio);
      break;

    case BAD_BOUNCINGSNOWBALL:
      action_bouncingsnowball(frame_ratio);
      break;

    case BAD_FLYINGSNOWBALL:
      action_flyingsnowball(frame_ratio);
      break;

    case BAD_SPIKY:
      action_spiky(frame_ratio);
      break;

    case BAD_SNOWBALL:
      action_snowball(frame_ratio);
      break;
    }
}

void
BadGuy::draw()
{
  // Don't try to draw stuff that is outside of the screen
  if(base.x <= scroll_x - base.width || base.x >= scroll_x + screen->w)
    return;
  if(texture_left == 0 || texture_right == 0)
    return;

  float global_frame = (float(global_frame_counter - animation_offset) / 10);
  global_frame *= animation_speed;
  size_t frame = size_t(global_frame) % animation_length;
  texture_type* texture = 
      (dir == LEFT) ? &texture_left[frame] : &texture_right[frame];
  texture_draw(texture, base.x - scroll_x, base.y);

  if (debug_mode)
    fillrect(base.x - scroll_x, base.y, 32, 32, 75,0,75, 150);
}

void
BadGuy::set_texture(texture_type* left, texture_type* right,
    int nanimlength, float nanimspeed)
{
  if(left != 0) {
    if(base.width == 0 && base.height == 0) {
      base.width = left->w;
      base.height = left->h;
    } else if(base.width != left->w || base.height != left->h) {
      base.x -= (left->w - base.width) / 2;
      base.y -= left->h - base.height;
      base.width = left->w;
      base.height = left->h;
      old_base = base;
    }
  } else {
    base.width = base.height = 0;
  }

  animation_length = nanimlength;
  animation_speed = nanimspeed;
  animation_offset = 0;
  texture_left = left;
  texture_right = right;
}

void
BadGuy::bump()
{
  if(kind == BAD_BSOD || kind == BAD_LAPTOP || kind == BAD_MRBOMB
      || kind == BAD_BOUNCINGSNOWBALL) {
    kill_me();
  }
}

void
BadGuy::make_player_jump(Player* player)
{
  player->physic.set_velocity(player->physic.get_velocity_x(), 2);
  player->base.y = base.y - player->base.height - 2;
}

void
BadGuy::squish_me(Player* player)
{
  make_player_jump(player);
    
  World::current()->add_score(base.x - scroll_x,
                              base.y, 50 * player_status.score_multiplier);
  play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
  player_status.score_multiplier++;

  dying = DYING_SQUISHED;
  timer.start(2000);
  physic.set_velocity(0, 0);
}

void
BadGuy::squish(Player* player)
{
  if(kind == BAD_MRBOMB) {
    // mrbomb transforms into a bomb now
    World::current()->add_bad_guy(base.x, base.y, BAD_BOMB);
    
    make_player_jump(player);
    World::current()->add_score(base.x - scroll_x, base.y, 50 * player_status.score_multiplier);
    play_sound(sounds[SND_SQUISH], SOUND_CENTER_SPEAKER);
    player_status.score_multiplier++;
      
    remove_me();
    return;

  } else if(kind == BAD_BSOD) {
    squish_me(player);
    set_texture(img_bsod_squished_left, img_bsod_squished_right, 1);
    physic.set_velocity(0, physic.get_velocity_y());
    return;
      
  } else if (kind == BAD_LAPTOP) {
    if (mode == NORMAL || mode == KICK)
      {
        /* Flatten! */
        play_sound(sounds[SND_STOMP], SOUND_CENTER_SPEAKER);
        mode = FLAT;
        set_texture(img_laptop_flat_left, img_laptop_flat_right, 1);
        physic.set_velocity(0, physic.get_velocity_y());

        timer.start(4000);
      } else if (mode == FLAT) {
        /* Kick! */
        play_sound(sounds[SND_KICK], SOUND_CENTER_SPEAKER);

        if (player->base.x < base.x + (base.width/2)) {
          physic.set_velocity(5, physic.get_velocity_y());
          dir = RIGHT;
        } else {
          physic.set_velocity(-5, physic.get_velocity_y());
          dir = LEFT;
        }

        mode = KICK;
        set_texture(img_laptop_flat_left, img_laptop_flat_right, 1);
      }

    make_player_jump(player);
	      
    World::current()->add_score(base.x - scroll_x, base.y, 25 * player_status.score_multiplier);
    player_status.score_multiplier++;
    return;
  } else if(kind == BAD_FISH) {
    make_player_jump(player);
	      
    World::current()->add_score(base.x - scroll_x, base.y, 25 * player_status.score_multiplier);
    player_status.score_multiplier++;
     
    // simply remove the fish...
    remove_me();
    return;
  } else if(kind == BAD_BOUNCINGSNOWBALL) {
    squish_me(player);
    set_texture(img_bouncingsnowball_squished,img_bouncingsnowball_squished,1);
    return;
  } else if(kind == BAD_FLYINGSNOWBALL) {
    squish_me(player);
    set_texture(img_flyingsnowball_squished,img_flyingsnowball_squished,1);
    return;
  } else if(kind == BAD_SNOWBALL) {
    squish_me(player);
    set_texture(img_snowball_squished_left, img_snowball_squished_right, 1);
    return;
  }
}

void
BadGuy::kill_me()
{
  if(kind == BAD_BOMB || kind == BAD_STALACTITE || kind == BAD_FLAME)
    return;

  dying = DYING_FALLING;
  if(kind == BAD_LAPTOP)
    set_texture(img_laptop_falling_left, img_laptop_falling_right, 1);
  else if(kind == BAD_BSOD)
    set_texture(img_bsod_falling_left, img_bsod_falling_right, 1);
  
  physic.enable_gravity(true);
  physic.set_velocity(physic.get_velocity_x(), 0);

  /* Gain some points: */
  if (kind == BAD_BSOD)
    World::current()->add_score(base.x - scroll_x, base.y,
                    50 * player_status.score_multiplier);
  else 
    World::current()->add_score(base.x - scroll_x, base.y,                                 
                    25 * player_status.score_multiplier);

  /* Play death sound: */
  play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
}

void
BadGuy::collision(void *p_c_object, int c_object, CollisionType type)
{
  BadGuy* pbad_c    = NULL;

  if(type == COLLISION_BUMP) {
    bump();
    return;
  }
  if(type == COLLISION_SQUISH) {
    Player* player = static_cast<Player*>(p_c_object);
    squish(player);
    return;
  }

  switch (c_object)
    {
    case CO_BULLET:
      kill_me();
      break;

    case CO_BADGUY:
      pbad_c = (BadGuy*) p_c_object;
      if(kind == BAD_LAPTOP && mode == KICK &&
            pbad_c->kind != BAD_FLAME && pbad_c->kind != BAD_BOMB)
        {
          /* We're in kick mode, kill the other guy
	     and yourself(wuahaha) : */
          pbad_c->kill_me();
          kill_me();
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

  texture_load(&img_bsod_squished_left[0], datadir +
               "/images/shared/bsod-squished-left.png",
               USE_ALPHA);

  texture_load(&img_bsod_squished_right[0], datadir +
               "/images/shared/bsod-squished-right.png",
               USE_ALPHA);

  texture_load(&img_bsod_falling_left[0], datadir +
               "/images/shared/bsod-falling-left.png",
               USE_ALPHA);

  texture_load(&img_bsod_falling_right[0], datadir +
               "/images/shared/bsod-falling-right.png",
               USE_ALPHA);


  /* (Laptop) */

  texture_load(&img_laptop_left[0], datadir + "/images/shared/mriceblock-left-0.png", USE_ALPHA);
  texture_load(&img_laptop_left[1], datadir + "/images/shared/mriceblock-left-1.png", USE_ALPHA);
  texture_load(&img_laptop_left[2], datadir + "/images/shared/mriceblock-left-2.png", USE_ALPHA);
  texture_load(&img_laptop_left[3], datadir + "/images/shared/mriceblock-left-1.png", USE_ALPHA);

  texture_load(&img_laptop_right[0], datadir + "/images/shared/mriceblock-right-0.png", USE_ALPHA);
  texture_load(&img_laptop_right[1], datadir + "/images/shared/mriceblock-right-1.png", USE_ALPHA);
  texture_load(&img_laptop_right[2], datadir + "/images/shared/mriceblock-right-2.png", USE_ALPHA);
  texture_load(&img_laptop_right[3], datadir + "/images/shared/mriceblock-right-1.png", USE_ALPHA);
  
  texture_load(&img_laptop_flat_left[0], 
               datadir + "/images/shared/laptop-flat-left.png",
               USE_ALPHA);

  texture_load(&img_laptop_flat_right[0], datadir +
               "/images/shared/laptop-flat-right.png",
               USE_ALPHA);

  texture_load(&img_laptop_falling_left[0], datadir +
               "/images/shared/laptop-falling-left.png",
               USE_ALPHA);

  texture_load(&img_laptop_falling_right[0], datadir +
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
  texture_load(&img_mrbomb_ticking_left[0],
          datadir + "/images/shared/mrbombx-left-0.png", USE_ALPHA);
  texture_load(&img_mrbomb_ticking_right[0],
          datadir + "/images/shared/mrbombx-right-0.png", USE_ALPHA);
  texture_load(&img_mrbomb_explosion[0],
          datadir + "/images/shared/mrbomb-explosion.png", USE_ALPHA);

  /* stalactite */
  texture_load(&img_stalactite[0], 
          datadir + "/images/shared/stalactite.png", USE_ALPHA);
  texture_load(&img_stalactite_broken[0],
          datadir + "/images/shared/stalactite-broken.png", USE_ALPHA);

  /* flame */
  texture_load(&img_flame[0],
          datadir + "/images/shared/flame-0.png", USE_ALPHA);
  texture_load(&img_flame[1],
          datadir + "/images/shared/flame-1.png", USE_ALPHA);  

  /* fish */
  texture_load(&img_fish[0],
          datadir + "/images/shared/fish-left-0.png", USE_ALPHA);
  texture_load(&img_fish[1],
          datadir + "/images/shared/fish-left-1.png", USE_ALPHA);

  /* bouncing snowball */
  for(int i=0; i<6; ++i) {
      char num[4];
      snprintf(num, 4, "%d", i);
      texture_load(&img_bouncingsnowball_left[i],
              datadir + "/images/shared/bouncingsnowball-left-" + num + ".png",
              USE_ALPHA);
      texture_load(&img_bouncingsnowball_right[i],
              datadir + "/images/shared/bouncingsnowball-right-" + num + ".png",
              USE_ALPHA);
  } 
  texture_load(&img_bouncingsnowball_squished[0],
          datadir + "/images/shared/bsod-squished-left.png", USE_ALPHA);

  /* flying snowball */
  texture_load(&img_flyingsnowball[0],
          datadir + "/images/shared/flyingsnowball-left-0.png", USE_ALPHA);
  texture_load(&img_flyingsnowball[1],
          datadir + "/images/shared/flyingsnowball-left-1.png", USE_ALPHA);  
  texture_load(&img_flyingsnowball_squished[0],
          datadir + "/images/shared/bsod-squished-left.png", USE_ALPHA);

  /* spiky */
  for(int i = 0; i < 3; ++i) {
        char num[4];
        snprintf(num, 4, "%d", i);
        texture_load(&img_spiky_left[i],                                 
                datadir + "/images/shared/spiky-left-" + num + ".png",   
                USE_ALPHA);
        texture_load(&img_spiky_right[i],
                datadir + "/images/shared/spiky-right-" + num + ".png",
                USE_ALPHA);
  }

  /** snowball */
  texture_load(&img_snowball_left[0], datadir + "/images/shared/snowball-left-0.png", USE_ALPHA);
  texture_load(&img_snowball_left[1], datadir + "/images/shared/snowball-left-1.png", USE_ALPHA);
  texture_load(&img_snowball_left[2], datadir + "/images/shared/snowball-left-2.png", USE_ALPHA);
  texture_load(&img_snowball_left[3], datadir + "/images/shared/snowball-left-1.png", USE_ALPHA);

  texture_load(&img_snowball_right[0], datadir + "/images/shared/snowball-right-0.png", USE_ALPHA);
  texture_load(&img_snowball_right[1], datadir + "/images/shared/snowball-right-1.png", USE_ALPHA);
  texture_load(&img_snowball_right[2], datadir + "/images/shared/snowball-right-2.png", USE_ALPHA);
  texture_load(&img_snowball_right[3], datadir + "/images/shared/snowball-right-1.png", USE_ALPHA);

  texture_load(&img_snowball_squished_left[0],
          datadir + "/images/shared/bsod-squished-left.png", USE_ALPHA);
  texture_load(&img_snowball_squished_right[0],
          datadir + "/images/shared/bsod-squished-right.png", USE_ALPHA);  
}

void free_badguy_gfx()
{
  for (int i = 0; i < 4; i++)
    {
      texture_free(&img_bsod_left[i]);
      texture_free(&img_bsod_right[i]);
    }

  texture_free(&img_bsod_squished_left[0]);
  texture_free(&img_bsod_squished_right[0]);

  texture_free(&img_bsod_falling_left[0]);
  texture_free(&img_bsod_falling_right[0]);

  for (int i = 0; i < 4; i++)
    {
      texture_free(&img_laptop_left[i]);
      texture_free(&img_laptop_right[i]);
    }

  texture_free(&img_laptop_flat_left[0]);
  texture_free(&img_laptop_flat_right[0]);

  texture_free(&img_laptop_falling_left[0]);
  texture_free(&img_laptop_falling_right[0]);

  for (int i = 0; i < 2; i++)
    {
      texture_free(&img_money_left[i]);
      texture_free(&img_money_right[i]);
    }

  for(int i = 0; i < 4; i++) {
      texture_free(&img_mrbomb_left[i]);
      texture_free(&img_mrbomb_right[i]);
  }

  texture_free(&img_mrbomb_ticking_left[0]);
  texture_free(&img_mrbomb_ticking_right[0]);
  texture_free(&img_mrbomb_explosion[0]);

  texture_free(&img_stalactite[0]);
  texture_free(&img_stalactite_broken[0]);

  texture_free(&img_flame[0]);
  texture_free(&img_flame[1]);

  texture_free(&img_fish[0]);
  texture_free(&img_fish[1]);

  for(int i=0; i<6; ++i) {
    texture_free(&img_bouncingsnowball_left[i]);
    texture_free(&img_bouncingsnowball_right[i]);
  }
  texture_free(&img_bouncingsnowball_squished[0]);

  texture_free(&img_flyingsnowball[0]);
  texture_free(&img_flyingsnowball[1]);
  texture_free(&img_flyingsnowball_squished[0]);

  for(int i = 0; i<3; ++i) {
    texture_free(&img_spiky_left[i]);
    texture_free(&img_spiky_right[i]);
  }
  for(int i = 0; i<4; ++i) {
    texture_free(&img_snowball_left[i]);
    texture_free(&img_snowball_right[i]);
  }
  texture_free(&img_snowball_squished_left[0]);
  texture_free(&img_snowball_squished_right[0]); 
}

// EOF //
