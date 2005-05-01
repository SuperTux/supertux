//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <typeinfo>
#include <cmath>
#include <iostream>
#include <cassert>

#include "app/globals.h"
#include "app/gettext.h"
#include "sprite/sprite_manager.h"
#include "player.h"
#include "tile.h"
#include "sprite/sprite.h"
#include "sector.h"
#include "resources.h"
#include "video/screen.h"
#include "statistics.h"
#include "game_session.h"
#include "object/tilemap.h"
#include "object/camera.h"
#include "object/gameobjs.h"
#include "object/portable.h"
#include "trigger/trigger_base.h"
#include "control/joystickkeyboardcontroller.h"
#include "main.h"

static const int TILES_FOR_BUTTJUMP = 3;
static const float SHOOTING_TIME = .150;
/// time before idle animation starts
static const float IDLE_TIME = 2.5;

static const float WALK_ACCELERATION_X = 300;
static const float RUN_ACCELERATION_X = 400;
static const float SKID_XM = 200;
static const float SKID_TIME = .3;
static const float MAX_WALK_XM = 230;
static const float MAX_RUN_XM = 320;
static const float WALK_SPEED = 100;

// growing animation
Surface* growingtux_left[GROWING_FRAMES];
Surface* growingtux_right[GROWING_FRAMES];

Surface* tux_life = 0;

TuxBodyParts* small_tux = 0;
TuxBodyParts* big_tux = 0;
TuxBodyParts* fire_tux = 0;
TuxBodyParts* ice_tux = 0;

void
TuxBodyParts::set_action(std::string action, int loops)
{
  if(head != NULL)
    head->set_action(action, loops);
  if(body != NULL)
    body->set_action(action, loops);
  if(arms != NULL)
    arms->set_action(action, loops);
  if(feet != NULL)
    feet->set_action(action, loops);
}

void
TuxBodyParts::draw(DrawingContext& context, const Vector& pos, int layer,
                  Uint32 drawing_effect)
{
  if(head != NULL)
    head->draw(context, pos, layer-1, drawing_effect);
  if(body != NULL)
    body->draw(context, pos, layer-3, drawing_effect);
  if(arms != NULL)
    arms->draw(context, pos, layer,   drawing_effect);
  if(feet != NULL)
    feet->draw(context, pos, layer-2, drawing_effect);
}

Player::Player(PlayerStatus* _player_status)
  : player_status(_player_status), grabbed_object(0)
{
  controller = main_controller;
  smalltux_gameover = sprite_manager->create("smalltux-gameover");
  smalltux_star = sprite_manager->create("smalltux-star");
  bigtux_star = sprite_manager->create("bigtux-star");
  init();
}

Player::~Player()
{
  delete smalltux_gameover;
  delete smalltux_star;
  delete bigtux_star;
}

void
Player::init()
{
  if(is_big())
    bbox.set_size(31.8, 63.8);
  else
    bbox.set_size(31.8, 31.8);

  dir = RIGHT;
  old_dir = dir;
  duck = false;
  dead = false;

  dying = false;
  last_ground_y = 0;
  fall_mode = ON_GROUND;
  jumping = false;
  flapping = false;
  can_jump = true;
  can_flap = false;
  falling_from_flap = false;
  enable_hover = false;
  butt_jump = false;
  
  flapping_velocity = 0;

  // temporary to help player's choosing a flapping
  flapping_mode = NO_FLAP;

  // Ricardo's flapping
  flaps_nb = 0;

  on_ground_flag = false;
  grabbed_object = 0;

  physic.reset();
}

void
Player::set_controller(Controller* controller)
{
  this->controller = controller;
}

void
Player::action(float elapsed_time)
{
  if(dying && dying_timer.check()) {
    dead = true;
    return;
  }

  if(!controller->hold(Controller::ACTION) && grabbed_object) {
    grabbed_object = 0;
    // move the grabbed object a bit away from tux
    Vector pos = get_pos() + 
        Vector(dir == LEFT ? -bbox.get_width() : bbox.get_width(),
                bbox.get_height()*0.66666 - 32);
    MovingObject* object = dynamic_cast<MovingObject*> (grabbed_object);
    if(object) {
      object->set_pos(pos);
    } else {
#ifdef DEBUG
      std::cout << "Non MovingObjetc grabbed?!?\n";
#endif
    }
  }

  if(!dying)
    handle_input();

  movement = physic.get_movement(elapsed_time);
  on_ground_flag = false;

#if 0
  // special exception for cases where we're stuck under tiles after
  // being ducked. In this case we drift out
  if(!duck && on_ground() && old_base.x == base.x && old_base.y == base.y
     && collision_object_map(base)) {
    base.x += elapsed_time * WALK_SPEED * (dir ? 1: -1);
    previous_base = old_base = base;
  }
#endif

  if(grabbed_object != 0) {
    Vector pos = get_pos() + 
      Vector(dir == LEFT ? -16 : 16,
             bbox.get_height()*0.66666 - 32);
    grabbed_object->grab(*this, pos);
  }
}

bool
Player::on_ground()
{
  return on_ground_flag;
}

bool
Player::is_big()
{
  if(player_status->bonus == NO_BONUS)
    return false;

  return true;
}

void
Player::handle_horizontal_input()
{
  float vx = physic.get_velocity_x();
  float vy = physic.get_velocity_y();
  float ax = physic.get_acceleration_x();
  float ay = physic.get_acceleration_y();

  float dirsign = 0;
  if(!duck || physic.get_velocity_y() != 0) {
    if(controller->hold(Controller::LEFT) && !controller->hold(Controller::RIGHT)) {
      old_dir = dir;
      dir = LEFT;
      dirsign = -1;
    } else if(!controller->hold(Controller::LEFT)
              && controller->hold(Controller::RIGHT)) {
      old_dir = dir;
      dir = RIGHT;
      dirsign = 1;
    }
  }

  if (!controller->hold(Controller::ACTION)) {
    ax = dirsign * WALK_ACCELERATION_X;
    // limit speed
    if(vx >= MAX_WALK_XM && dirsign > 0) {
      vx = MAX_WALK_XM;
      ax = 0;
    } else if(vx <= -MAX_WALK_XM && dirsign < 0) {
      vx = -MAX_WALK_XM;
      ax = 0;
    }
  } else {
    ax = dirsign * RUN_ACCELERATION_X;
    // limit speed
    if(vx >= MAX_RUN_XM && dirsign > 0) {
      vx = MAX_RUN_XM;
      ax = 0;
    } else if(vx <= -MAX_RUN_XM && dirsign < 0) {
      vx = -MAX_RUN_XM;
      ax = 0;
    }
  }

  // we can reach WALK_SPEED without any acceleration
  if(dirsign != 0 && fabs(vx) < WALK_SPEED) {
    vx = dirsign * WALK_SPEED;
  }

  // changing directions?
  if(on_ground() && ((vx < 0 && dirsign >0) || (vx>0 && dirsign<0))) {
    // let's skid!
    if(fabs(vx)>SKID_XM && !skidding_timer.started()) {
      skidding_timer.start(SKID_TIME);
      sound_manager->play_sound("skid");
      // dust some partcles
      Sector::current()->add_object(
        new Particles(
          Vector(bbox.p1.x + (dir == RIGHT ? bbox.get_width() : 0),
                 bbox.p2.y),
          dir == RIGHT ? 270+20 : 90-40, dir == RIGHT ? 270+40 : 90-20,
          Vector(280,-260), Vector(0,0.030), 3, Color(100,100,100), 3, .8,
          LAYER_OBJECTS+1));
      
      ax *= 2.5;
    } else {
      ax *= 2;
    }
  }

  // we get slower when not pressing any keys
  if(dirsign == 0) {
    if(fabs(vx) < WALK_SPEED) {
      vx = 0;
      ax = 0;
    } else if(vx < 0) {
      ax = WALK_ACCELERATION_X * 1.5;
    } else {
      ax = WALK_ACCELERATION_X * -1.5;
    }
  }

#if 0
  // if we're on ice slow down acceleration or deceleration
  if (isice(base.x, base.y + base.height))
  {
    /* the acceleration/deceleration rate on ice is inversely proportional to
     * the current velocity.
     */

    // increasing 1 will increase acceleration/deceleration rate
    // decreasing 1 will decrease acceleration/deceleration rate
    //  must stay above zero, though
    if (ax != 0) ax *= 1 / fabs(vx);
  }
#endif

  // extend/shrink tux collision rectangle so that we fall through/walk over 1
  // tile holes
  if(fabsf(vx) > MAX_WALK_XM) {
    bbox.set_width(33);
  } else {
    bbox.set_width(31.8);
  }

  physic.set_velocity(vx, vy);
  physic.set_acceleration(ax, ay);
}

void
Player::handle_vertical_input()
{
  // set fall mode...
  if(on_ground()) {
    fall_mode = ON_GROUND;
    last_ground_y = get_pos().y;
  } else {
    if(get_pos().y > last_ground_y)
      fall_mode = FALLING;
    else if(fall_mode == ON_GROUND)
      fall_mode = JUMPING;
  }

  if(on_ground()) { /* Make sure jumping is off. */
    jumping = false;
    flapping = false;
    falling_from_flap = false;
    if (flapping_timer.started()) {
      flapping_timer.start(0);
    }

    physic.set_acceleration_y(0); //for flapping
  }

  // Press jump key
  if(controller->pressed(Controller::JUMP) && can_jump && on_ground()) {
    if(duck) { // only jump a little bit when in duck mode {
      physic.set_velocity_y(300);
    } else {
      // jump higher if we are running
      if (fabs(physic.get_velocity_x()) > MAX_WALK_XM)
        physic.set_velocity_y(580);
      else
        physic.set_velocity_y(520);
    }
    
    //bbox.move(Vector(0, -1));
    jumping = true;
    flapping = false;
    can_jump = false;
    can_flap = false;
    flaps_nb = 0; // Ricardo's flapping
    if (is_big())
      sound_manager->play_sound("bigjump");
    else
      sound_manager->play_sound("jump");
  } else if(!controller->hold(Controller::JUMP)) { // Let go of jump key
    if (!flapping && !duck && !falling_from_flap && !on_ground()) {
      can_flap = true;
    }
    if (jumping && physic.get_velocity_y() > 0) {
      jumping = false;
      physic.set_velocity_y(0);
    }
  }

  // temporary to help player's choosing a flapping
  if(flapping_mode == RICARDO_FLAP) {
    // Flapping, Ricardo's version
    // similar to SM3 Fox
    if(controller->pressed(Controller::JUMP) && can_flap && flaps_nb < 3) {
      physic.set_velocity_y(350);
      physic.set_velocity_x(physic.get_velocity_x() * 35);
      flaps_nb++;
    }
  } else if(flapping_mode == MAREK_FLAP) {
    // Flapping, Marek's version
    if (controller->hold(Controller::JUMP) && can_flap)
    {
      if (!flapping_timer.started())
      {
        flapping_timer.start(TUX_FLAPPING_TIME);
        flapping_velocity = physic.get_velocity_x();
      }
      if (flapping_timer.check()) 
      {
        can_flap = false;
        falling_from_flap = true;
      }
      jumping = true;
      flapping = true;
      if (!flapping_timer.check()) {
        float cv = flapping_velocity * sqrt(
          TUX_FLAPPING_TIME - flapping_timer.get_timegone() 
          / TUX_FLAPPING_TIME);
        
        //Handle change of direction while flapping
        if (((dir == LEFT) && (cv > 0)) || (dir == RIGHT) && (cv < 0)) {
          cv *= (-1);
        }
        physic.set_velocity_x(cv);
        physic.set_velocity_y(
          flapping_timer.get_timegone()/.850);
      }
    }
  } else if(flapping_mode == RYAN_FLAP) {
    // Flapping, Ryan's version
    if (controller->hold(Controller::JUMP) && can_flap)
    {
      if (!flapping_timer.started())
      {
        flapping_timer.start(TUX_FLAPPING_TIME);
      }
      if (flapping_timer.check()) 
      {
        can_flap = false;
        falling_from_flap = true;
      }
      jumping = true;
      flapping = true;
      if (flapping && flapping_timer.get_timegone() <= TUX_FLAPPING_TIME
          && physic.get_velocity_y() < 0)
      {
        float gravity = Sector::current()->gravity;
        (void)gravity;
        float xr = (fabsf(physic.get_velocity_x()) / MAX_RUN_XM);
        
        // XXX: magic numbers. should be a percent of gravity
        //      gravity is (by default) -0.1f
        physic.set_acceleration_y(12 + 1*xr);
        
#if 0
        // To slow down x-vel when flapping (not working)
        if (fabsf(physic.get_velocity_x()) > MAX_WALK_XM)
        {
          if (physic.get_velocity_x() < 0)
            physic.set_acceleration_x(1.0f);
          else if (physic.get_velocity_x() > 0)
            physic.set_acceleration_x(-1.0f);
        }
#endif
      }
    } else {
      physic.set_acceleration_y(0);
    }
  }

  /* In case the player has pressed Down while in a certain range of air,
     enable butt jump action */
  if (controller->hold(Controller::DOWN) && !butt_jump && !duck)
    //if(tiles_on_air(TILES_FOR_BUTTJUMP) && jumping)
    butt_jump = true;
  
  /* When Down is not held anymore, disable butt jump */
  if(butt_jump && !controller->hold(Controller::DOWN))
    butt_jump = false;
  
#if 0
  // Do butt jump
  if (butt_jump && on_ground() && is_big()) {
    // Add a smoke cloud
    if (duck) 
      Sector::current()->add_smoke_cloud(Vector(get_pos().x - 32, get_pos().y));
    else 
      Sector::current()->add_smoke_cloud(
        Vector(get_pos().x - 32, get_pos().y + 32));
    
    butt_jump = false;
    
    // Break bricks beneath Tux
    if(Sector::current()->trybreakbrick(
         Vector(base.x + 1, base.y + base.height), false)
       || Sector::current()->trybreakbrick(
         Vector(base.x + base.width - 1, base.y + base.height), false)) {
      physic.set_velocity_y(2);
      butt_jump = true;
    }
    
    // Kill nearby badguys
    std::vector<GameObject*> gameobjects = Sector::current()->gameobjects;
    for (std::vector<GameObject*>::iterator i = gameobjects.begin();
         i != gameobjects.end();
         i++) {
      BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
      if(badguy) {
        // don't kill when badguys are already dying or in a certain mode
        if(badguy->dying == DYING_NOT && badguy->mode != BadGuy::BOMB_TICKING &&
           badguy->mode != BadGuy::BOMB_EXPLODE) {
          if (fabsf(base.x - badguy->base.x) < 96 &&
              fabsf(base.y - badguy->base.y) < 64)
            badguy->kill_me(25);
        }
      }
    }
  }
#endif

  /** jumping is only allowed if we're about to touch ground soon and if the
   * button has been up in between the last jump
   */
  // FIXME
#if 0
  if ( (issolid(get_pos().x + bbox.get_width() / 2,
          get_pos().y + bbox.get_height() + 64) ||
        issolid(get_pos().x + 1, get_pos().y + bbox.get_height() + 64) ||
        issolid(get_pos().x + bbox.get_width() - 1,
          get_pos().y + bbox.get_height() + 64))
       && jumping  == false
       && can_jump == false
       && input.jump && !input.old_jump)
    {
      can_jump = true;
    }
#endif
}

void
Player::handle_input()
{
  /* Handle horizontal movement: */
  handle_horizontal_input();

  /* Jump/jumping? */
  if (on_ground() && !controller->hold(Controller::JUMP))
    can_jump = true;
  handle_vertical_input();

  /* Shoot! */
  if (controller->pressed(Controller::ACTION) && player_status->bonus == FIRE_BONUS) {
    if(Sector::current()->add_bullet(
         get_pos() + ((dir == LEFT)? Vector(0, bbox.get_height()/2) 
                      : Vector(32, bbox.get_height()/2)),
         physic.get_velocity_x(), dir))
      shooting_timer.start(SHOOTING_TIME);
  }
  
  /* Duck! */
  if (controller->hold(Controller::DOWN) && is_big() && !duck 
      && physic.get_velocity_y() == 0 && on_ground()) {
    duck = true;
    bbox.move(Vector(0, 32));
    bbox.set_height(31.8);
  } else if(!controller->hold(Controller::DOWN) && is_big() && duck) {
    // try if we can really unduck
    bbox.move(Vector(0, -32));
    bbox.set_height(63.8);
    duck = false;
    // FIXME
#if 0
    // when unducking in air we need some space to do so
    if(on_ground() || !collision_object_map(bbox)) {
      duck = false;
    } else {
      // undo the ducking changes
      bbox.move(Vector(0, 32));
      bbox.set_height(31.8);
    }
#endif
  }
}

void
Player::set_bonus(BonusType type, bool animate)
{
  if(player_status->bonus == type)
    return;
  
  if(player_status->bonus == NO_BONUS) {
    bbox.set_height(63.8);
    bbox.move(Vector(0, -32));
    if(animate)
      growing_timer.start(GROWING_TIME);
  }
  
  player_status->bonus = type;
}

void
Player::draw(DrawingContext& context)
{
  TuxBodyParts* tux_body;
          
  if (player_status->bonus == GROWUP_BONUS)
    tux_body = big_tux;
  else if (player_status->bonus == FIRE_BONUS)
    tux_body = fire_tux;
  else if (player_status->bonus == ICE_BONUS)
    tux_body = ice_tux;
  else
    tux_body = small_tux;

  int layer = LAYER_OBJECTS + 10;

  /* Set Tux sprite action */
  if (duck && is_big())
    {
    if(dir == LEFT)
      tux_body->set_action("duck-left");
    else // dir == RIGHT
      tux_body->set_action("duck-right");
    }
  else if (skidding_timer.started() && !skidding_timer.check())
    {
    if(dir == LEFT)
      tux_body->set_action("skid-left");
    else // dir == RIGHT
      tux_body->set_action("skid-right");
    }
  else if (kick_timer.started() && !kick_timer.check())
    {
    if(dir == LEFT)
      tux_body->set_action("kick-left");
    else // dir == RIGHT
      tux_body->set_action("kick-right");
    }
  else if (butt_jump && is_big())
    {
    if(dir == LEFT)
      tux_body->set_action("buttjump-left");
    else // dir == RIGHT
      tux_body->set_action("buttjump-right");
    }
  else if (physic.get_velocity_y() != 0)
    {
    if(dir == LEFT)
      tux_body->set_action("jump-left");
    else // dir == RIGHT
      tux_body->set_action("jump-right");
    }
  else
    {
    if (fabsf(physic.get_velocity_x()) < 1.0f) // standing
      {
      if(dir == LEFT)
        tux_body->set_action("stand-left");
      else // dir == RIGHT
        tux_body->set_action("stand-right");
      }
    else // moving
      {
      if(dir == LEFT)
        tux_body->set_action("walk-left");
      else // dir == RIGHT
        tux_body->set_action("walk-right");
      }
    }

  if(idle_timer.check())
    {
    if(is_big())
      {
      if(dir == LEFT)
        tux_body->head->set_action("idle-left", 1);
      else // dir == RIGHT
        tux_body->head->set_action("idle-right", 1);
      }

    }

  // Tux is holding something
  if ((grabbed_object != 0 && physic.get_velocity_y() == 0) ||
      (shooting_timer.get_timeleft() > 0 && !shooting_timer.check()))
    {
    if (duck)
      {
      if(dir == LEFT)
        tux_body->arms->set_action("duck+grab-left");
      else // dir == RIGHT
        tux_body->arms->set_action("duck+grab-right");
      }
    else
      {
      if(dir == LEFT)
        tux_body->arms->set_action("grab-left");
      else // dir == RIGHT
        tux_body->arms->set_action("grab-right");
      }
    }

  /* Draw Tux */
  if(dying) {
    smalltux_gameover->draw(context, get_pos(), layer);
  } else if(growing_timer.get_timeleft() > 0) {
    if(!is_big())
      {
      if (dir == RIGHT)
        context.draw_surface(growingtux_right[GROWING_FRAMES-1 - 
                 int((growing_timer.get_timegone() *
                 GROWING_FRAMES) / GROWING_TIME)], get_pos(), layer);
      else
        context.draw_surface(growingtux_left[GROWING_FRAMES-1 - 
                int((growing_timer.get_timegone() *
                GROWING_FRAMES) / GROWING_TIME)], get_pos(), layer);
      }
    else
      {
      if (dir == RIGHT)
        context.draw_surface(growingtux_right[
            int((growing_timer.get_timegone() *
                GROWING_FRAMES) / GROWING_TIME)], get_pos(), layer);
      else
        context.draw_surface(growingtux_left[
            int((growing_timer.get_timegone() *
                             GROWING_FRAMES) / GROWING_TIME)],
            get_pos(), layer);
      }
    }
  else if (safe_timer.started() && size_t(global_time*40)%2)
    ;  // don't draw Tux
  else
    tux_body->draw(context, get_pos(), layer);

  // Draw blinking star overlay
  if (invincible_timer.started() &&
     (invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING
      || size_t(global_time*20)%2)
     && !dying)
  {
    if (!is_big() || duck)
      smalltux_star->draw(context, get_pos(), layer + 5);
    else
      bigtux_star->draw(context, get_pos(), layer + 5);
  } 
}

HitResponse
Player::collision(GameObject& other, const CollisionHit& hit)
{
  Portable* portable = dynamic_cast<Portable*> (&other);
  if(portable && grabbed_object == 0 && controller->hold(Controller::ACTION)
     && fabsf(hit.normal.x) > .9) {
    grabbed_object = portable;
    return CONTINUE;
  }
 
  if(other.get_flags() & FLAG_SOLID) {
    if(hit.normal.y < 0) { // landed on floor?
      if (physic.get_velocity_y() < 0)
        physic.set_velocity_y(0);
      on_ground_flag = true;
    } else if(hit.normal.y > 0) { // bumped against the roof
      physic.set_velocity_y(.1);
    }
    
    if(fabsf(hit.normal.x) > .9) { // hit on the side?
      physic.set_velocity_x(0);
    }

    return CONTINUE;
  }

  TriggerBase* trigger = dynamic_cast<TriggerBase*> (&other);
  if(trigger) {
    if(controller->pressed(Controller::UP))
      trigger->event(*this, TriggerBase::EVENT_ACTIVATE);
  }

  return FORCE_MOVE;
}

void
Player::make_invincible()
{
  sound_manager->play_sound("invincible");
  invincible_timer.start(TUX_INVINCIBLE_TIME);
  Sector::current()->play_music(HERRING_MUSIC);               
}

/* Kill Player! */
void
Player::kill(HurtMode mode)
{
  if(dying)
    return;

  if(mode != KILL && 
          safe_timer.get_timeleft() > 0 || invincible_timer.get_timeleft() > 0)
    return;                          
  
  sound_manager->play_sound("hurt");

  physic.set_velocity_x(0);

  if (mode == SHRINK && is_big())
    {
      if (player_status->bonus == FIRE_BONUS
          || player_status->bonus == ICE_BONUS)
        {
          safe_timer.start(TUX_SAFE_TIME);
          player_status->bonus = GROWUP_BONUS;
        }
      else 
        {
          growing_timer.start(GROWING_TIME);
          safe_timer.start(TUX_SAFE_TIME + GROWING_TIME);
          bbox.set_height(31.8);
          duck = false;
          player_status->bonus = NO_BONUS;
        }
    }
  else
    {
      physic.enable_gravity(true);
      physic.set_acceleration(0, 0);
      physic.set_velocity(0, 700);
      player_status->lives -= 1;
      player_status->bonus = NO_BONUS;
      dying = true;
      dying_timer.start(3.0);
      flags |= FLAG_NO_COLLDET;
    }
}

void
Player::move(const Vector& vector)
{
  bbox.set_pos(vector);
  if(is_big())
    bbox.set_size(31.8, 63.8);
  else
    bbox.set_size(31.8, 31.8);
  on_ground_flag = false;
  duck = false;
  last_ground_y = vector.y;

  physic.reset();
}

void
Player::check_bounds(Camera* camera)
{
  /* Keep tux in bounds: */
  if (get_pos().x < 0)
    { // Lock Tux to the size of the level, so that he doesn't fall of
      // on the left side
      bbox.set_pos(Vector(0, get_pos().y));
    }

  /* Keep in-bounds, vertically: */
  if (get_pos().y > Sector::current()->solids->get_height() * 32)
    {
      kill(KILL);
      return;
    }

  bool adjust = false;
  // can happen if back scrolling is disabled
  if(get_pos().x < camera->get_translation().x) {
    bbox.set_pos(Vector(camera->get_translation().x, get_pos().y));
    adjust = true;
  }
  if(get_pos().x >= camera->get_translation().x + SCREEN_WIDTH - bbox.get_width())
  {
    bbox.set_pos(Vector(
          camera->get_translation().x + SCREEN_WIDTH - bbox.get_width(),
          get_pos().y));
    adjust = true;
  }

  if(adjust) {
    // FIXME
#if 0
    // squished now?
    if(collision_object_map(bbox)) {
      kill(KILL);
      return;
    }
#endif
  }
}

void
Player::bounce(BadGuy& )
{
  //Make sure we stopped flapping
  flapping = false;
  falling_from_flap = false;
  
  if(controller->hold(Controller::JUMP))
    physic.set_velocity_y(520);
  else
    physic.set_velocity_y(200);
}

