//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "gettext.hpp"
#include "sprite/sprite_manager.hpp"
#include "audio/sound_manager.hpp"
#include "player.hpp"
#include "tile.hpp"
#include "sprite/sprite.hpp"
#include "sector.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "game_session.hpp"
#include "object/tilemap.hpp"
#include "object/camera.hpp"
#include "object/particles.hpp"
#include "object/portable.hpp"
#include "object/bullet.hpp"
#include "trigger/trigger_base.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "scripting/wrapper_util.hpp"
#include "main.hpp"
#include "platform.hpp"
#include "badguy/badguy.hpp"
#include "player_status.hpp"
#include "log.hpp"
#include "falling_coin.hpp"

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

static const float KICK_TIME = .3;

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
TuxBodyParts::draw(DrawingContext& context, const Vector& pos, int layer)
{
  if(head != NULL)
    head->draw(context, pos, layer-1);
  if(body != NULL)
    body->draw(context, pos, layer-3);
  if(arms != NULL)
    arms->draw(context, pos, layer);
  if(feet != NULL)
    feet->draw(context, pos, layer-2);
}

Player::Player(PlayerStatus* _player_status)
  : player_status(_player_status), grabbed_object(0)
{
  controller = main_controller;
  smalltux_gameover = sprite_manager->create("images/creatures/tux_small/smalltux-gameover.sprite");
  smalltux_star = sprite_manager->create("images/creatures/tux_small/smalltux-star.sprite");
  bigtux_star = sprite_manager->create("images/creatures/tux_big/bigtux-star.sprite");

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
    bbox.set_size(31.8, 62.8);
  else
    bbox.set_size(31.8, 30.8);
  adjust_height = 0;

  dir = RIGHT;
  old_dir = dir;
  duck = false;
  dead = false;

  dying = false;
  last_ground_y = 0;
  fall_mode = ON_GROUND;
  jumping = false;
  can_jump = true;
  butt_jump = false;
  deactivated = false;
  backflipping = false;
  backflip_direction = 0;
  visible = true;
  
  on_ground_flag = false;
  grabbed_object = 0;

  floor_normal = Vector(0,-1);

  physic.reset();
}

void
Player::expose(HSQUIRRELVM vm, int table_idx)
{
  Scripting::Player* interface = static_cast<Scripting::Player*> (this);
  expose_object(vm, table_idx, interface, "Tux", false);
}

void
Player::unexpose(HSQUIRRELVM vm, int table_idx)
{
  Scripting::unexpose_object(vm, table_idx, "Tux");
}

void
Player::set_controller(Controller* controller)
{
  this->controller = controller;
}

void
Player::update(float elapsed_time)
{
  if(dying && dying_timer.check()) {
    dead = true;
    return;
  }

  if(adjust_height != 0) {
    bbox.move(Vector(0, bbox.get_height() - adjust_height));
    bbox.set_height(adjust_height);
    adjust_height = 0;
  }

  if(!controller->hold(Controller::ACTION) && grabbed_object) {
    // move the grabbed object a bit away from tux
    Vector pos = get_pos() + 
        Vector(dir == LEFT ? -bbox.get_width()-1 : bbox.get_width()+1,
                bbox.get_height()*0.66666 - 32);
    Rect dest(pos, pos + Vector(32, 32));
    if(Sector::current()->is_free_space(dest)) {
      MovingObject* moving_object = dynamic_cast<MovingObject*> (grabbed_object);
      if(moving_object) {
        moving_object->set_pos(pos);
      } else {
        log_debug << "Non MovingObjetc grabbed?!?" << std::endl;
      }
      grabbed_object->ungrab(*this, dir);
      grabbed_object = 0;
    }
  }

  if(!dying && !deactivated)
    handle_input();

  movement = physic.get_movement(elapsed_time);

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
    grabbed_object->grab(*this, pos, dir);
  }

  on_ground_flag = false;
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
      sound_manager->play("sounds/skid.wav");
      // dust some particles
      Sector::current()->add_object(
        new Particles(
          Vector(dir == RIGHT ? bbox.p2.x : bbox.p1.x, bbox.p2.y),
          dir == RIGHT ? 270+20 : 90-40, dir == RIGHT ? 270+40 : 90-20,
          Vector(280, -260), Vector(0, 300), 3, Color(.4, .4, .4), 3, .8,
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
    bbox.set_width(34);
  } else {
    bbox.set_width(31.8);
  }

  // on downward slopes, adjust vertical velocity to match slope angle
  if (on_ground()) {
    if (floor_normal.y != 0) {
      if ((floor_normal.x * vx) > 0) {
        // we overdo it a little, just to be on the safe side
        vy = vx * (floor_normal.x / floor_normal.y) * 2;
      }
    }
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
    if (backflipping) {
      backflipping = false;
      backflip_direction = 0;
    }
  }

  // Press jump key
  if(controller->pressed(Controller::JUMP) && can_jump && on_ground()) {
    if (duck) { 
      if (physic.get_velocity_x() != 0) // only jump a little bit when running ducked
        physic.set_velocity_y(300);
      else { //do a backflip
        backflipping = true;
        physic.set_velocity_y(580);
        backflip_timer.start(0.15);
      }
    }
    else if (fabs(physic.get_velocity_x()) > MAX_WALK_XM) // jump higher if we are running
      physic.set_velocity_y(580);
    else
      physic.set_velocity_y(520);
    
    //bbox.move(Vector(0, -1));
    jumping = true;
    can_jump = false;
    if (is_big())
      sound_manager->play("sounds/bigjump.wav");
    else
      sound_manager->play("sounds/jump.wav");
  } else if(!controller->hold(Controller::JUMP)) { // Let go of jump key
    if (!backflipping && jumping && physic.get_velocity_y() > 0) {
      jumping = false;
      physic.set_velocity_y(0);
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
  if (!backflipping) handle_horizontal_input();
  else {
    if (backflip_direction == 0) {
      dir == LEFT ? backflip_direction = 1 : backflip_direction = -1;
    }
    else backflip_direction == 1 ? dir = LEFT : dir = RIGHT; //prevent player from changing direction when backflipping 
    if (backflip_timer.check()) physic.set_velocity_x(100 * backflip_direction);
  }


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
    // if we have some velocity left then check if there is space for
    // unducking
    bbox.move(Vector(0, -32));
    bbox.set_height(63.8);
    if(Sector::current()->is_free_space(bbox) || (
        physic.get_velocity_x() > -.01 && physic.get_velocity_x() < .01
        && physic.get_velocity_y() > -.01 && physic.get_velocity_y() < .01))
    {
      duck = false;
    } else {
      // undo the ducking changes
      bbox.move(Vector(0, 32));
      bbox.set_height(31.8); 
    }
  }
}

void
Player::add_coins(int count)
{
  player_status->add_coins(count);
}

void
Player::add_bonus(const std::string& bonustype)
{
  if(bonustype == "grow")
    add_bonus(GROWUP_BONUS);
  else if(bonustype == "fireflower")
    add_bonus(FIRE_BONUS);
  else if(bonustype == "iceflower")
    add_bonus(ICE_BONUS);
  else if(bonustype == "none")
    add_bonus(NO_BONUS);
  
  
  std::ostringstream msg;
  msg << "Unknown bonus type "  << bonustype;
  throw std::runtime_error(msg.str());
}

void
Player::add_bonus(BonusType type, bool animate)
{
  // always ignore NO_BONUS
  if (type == NO_BONUS) {
    return;
  }

  // ignore GROWUP_BONUS if we're already big
  if (type == GROWUP_BONUS) {
    if (player_status->bonus == GROWUP_BONUS) return; 
    if (player_status->bonus == FIRE_BONUS) return;
    if (player_status->bonus == ICE_BONUS) return;
  }

  set_bonus(type, animate);
}

void
Player::set_bonus(BonusType type, bool animate)
{
  if(player_status->bonus == NO_BONUS) {
    adjust_height = 62.8;
    if(animate)
      growing_timer.start(GROWING_TIME);
  }

  if ((type == NO_BONUS) || (type == GROWUP_BONUS)) {
    player_status->max_fire_bullets = 0;
    player_status->max_ice_bullets = 0;
  }
  if (type == FIRE_BONUS) player_status->max_fire_bullets++;
  if (type == ICE_BONUS) player_status->max_ice_bullets++;

  player_status->bonus = type;
}

void
Player::set_visible(bool visible)
{
  this->visible = visible;
}

bool
Player::get_visible()
{
  return visible;
}

void
Player::kick()
{
  kick_timer.start(KICK_TIME);
}

void
Player::draw(DrawingContext& context)
{
  if(!visible)
    return;

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
  else if (!on_ground())
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
      if (dir == RIGHT) {
        context.draw_surface(growingtux_right[int((growing_timer.get_timegone() *
                 GROWING_FRAMES) / GROWING_TIME)], get_pos() - Vector(0, 32), layer);
      } else {
        context.draw_surface(growingtux_left[int((growing_timer.get_timegone() *
                GROWING_FRAMES) / GROWING_TIME)], get_pos() - Vector(0, 32), layer);
      }
    }
  else if (safe_timer.started() && size_t(game_time*40)%2)
    ;  // don't draw Tux
  else
    tux_body->draw(context, get_pos(), layer);

  // Draw blinking star overlay
  if (invincible_timer.started() &&
     (invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING
      || size_t(game_time*20)%2)
     && !dying)
  {
    if (!is_big() || duck)
      smalltux_star->draw(context, get_pos(), layer + 5);
    else
      bigtux_star->draw(context, get_pos(), layer + 5);
  } 
}

void
Player::collision_tile(uint32_t tile_attributes)
{
  if(tile_attributes & Tile::HURTS)
    kill(SHRINK);
}

HitResponse
Player::collision(GameObject& other, const CollisionHit& hit)
{
  Bullet* bullet = dynamic_cast<Bullet*> (&other);
  if(bullet) {
    return FORCE_MOVE;
  }

  if(other.get_flags() & FLAG_PORTABLE) {
    Portable* portable = dynamic_cast<Portable*> (&other);
    if(portable && grabbed_object == 0 && controller->hold(Controller::ACTION)
        && fabsf(hit.normal.x) > .9) {
      grabbed_object = portable;
      return CONTINUE;
    }
  }
 
  if(other.get_flags() & FLAG_SOLID) {
    /*
    printf("Col %p: HN: %3.1f %3.1f D %.1f P: %3.1f %3.1f M: %3.1f %3.1f\n",
        &other,
        hit.normal.x, hit.normal.y, hit.depth,
        get_pos().x, get_pos().y,
        movement.x, movement.y);
    */
    
    if(hit.normal.y < 0) { // landed on floor?
      if(physic.get_velocity_y() < 0)
        physic.set_velocity_y(0);

      on_ground_flag = true;

      // remember normal of this tile
      if (hit.normal.y > -0.9) {
        floor_normal.x = hit.normal.x;
        floor_normal.y = hit.normal.y;
      } else {
        // slowly adjust to unisolid tiles. 
        // Necessary because our bounding box sometimes reaches through slopes and thus hits unisolid tiles
        floor_normal.x = (floor_normal.x * 0.9) + (hit.normal.x * 0.1);
        floor_normal.y = (floor_normal.y * 0.9) + (hit.normal.y * 0.1);
      }

      // hack platforms so that we stand normally on them when going down...
      Platform* platform = dynamic_cast<Platform*> (&other);
      if(platform != NULL) {
        if(platform->get_speed().y > 0)
          physic.set_velocity_y(-platform->get_speed().y);
        //physic.set_velocity_x(platform->get_speed().x);
      }
    } else if(hit.normal.y > 0) { // bumped against the roof
      physic.set_velocity_y(.1);

      // hack platform so that we are not glued to it from below
      Platform* platform = dynamic_cast<Platform*> (&other);
      if(platform != NULL) {
        physic.set_velocity_y(-platform->get_speed().y);
      }      
    }
    
    if(fabsf(hit.normal.x) > .9) { // hit on the side?
      physic.set_velocity_x(0);
    }

    MovingObject* omov = dynamic_cast<MovingObject*> (&other);
    if(omov != NULL) {
      Vector mov = movement - omov->get_movement();
      /*
      printf("W %p - HITN: %3.1f %3.1f D:%3.1f TM: %3.1f %3.1f TD: %3.1f %3.1f PM: %3.2f %3.1f\n",
          omov,
          hit.normal.x, hit.normal.y,
          hit.depth,
          movement.x, movement.y,
          dest.p1.x, dest.p1.y,
          omov->get_movement().x, omov->get_movement().y);
      */
    }
    
    return CONTINUE;
  }

#ifdef DEBUG
  assert(dynamic_cast<MovingObject*> (&other) != NULL);
#endif
  MovingObject* moving_object = static_cast<MovingObject*> (&other); 
  if(moving_object->get_group() == COLGROUP_TOUCHABLE) {
    TriggerBase* trigger = dynamic_cast<TriggerBase*> (&other);
    if(trigger) {
      if(controller->pressed(Controller::UP))
        trigger->event(*this, TriggerBase::EVENT_ACTIVATE);
    }

    return FORCE_MOVE;
  }

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy != NULL) {
    if(safe_timer.started())
      return FORCE_MOVE;

    return CONTINUE;
  }

  return FORCE_MOVE;
}

void
Player::make_invincible()
{
  sound_manager->play("sounds/invincible.wav");
  invincible_timer.start(TUX_INVINCIBLE_TIME);
  Sector::current()->play_music(HERRING_MUSIC);               
}

/* Kill Player! */
void
Player::kill(HurtMode mode)
{
  if(dying || deactivated)
    return;

  if(mode != KILL && 
          (safe_timer.get_timeleft() > 0 || invincible_timer.get_timeleft() > 0))
    return;                          
  
  sound_manager->play("sounds/hurt.wav");

  physic.set_velocity_x(0);

  if (mode == SHRINK && is_big())
    {
      if (player_status->bonus == FIRE_BONUS
          || player_status->bonus == ICE_BONUS)
        {
          safe_timer.start(TUX_SAFE_TIME);
	  set_bonus(GROWUP_BONUS);
        }
      else 
        {
          //growing_timer.start(GROWING_TIME);
          safe_timer.start(TUX_SAFE_TIME /* + GROWING_TIME */);
          adjust_height = 30.8;
          duck = false;
          set_bonus(NO_BONUS);
        }
    }
  else
    {
      srand(time(0));
      int i;
      for (i = 0; (i < 5) && (i < player_status->coins); i++)
      {
        // the numbers: starting x, starting y, velocity y
        Sector::current()->add_object(new FallingCoin(get_pos() + Vector(rand()%5, rand()%50 - 32), rand()%200 - 100));
      }
      physic.enable_gravity(true);
      physic.set_acceleration(0, 0);
      physic.set_velocity(0, 700);
      player_status->coins -= 25;
      set_bonus(NO_BONUS);
      dying = true;
      dying_timer.start(3.0);
      set_group(COLGROUP_DISABLED);

      DisplayEffect* effect = new DisplayEffect();
      effect->fade_out(3.0);
      Sector::current()->add_object(effect);
      sound_manager->stop_music(3.0);
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
Player::add_velocity(const Vector& velocity)
{
  physic.set_velocity(physic.get_velocity() + velocity);
}

void
Player::bounce(BadGuy& )
{
  if(controller->hold(Controller::JUMP))
    physic.set_velocity_y(520);
  else
    physic.set_velocity_y(300);
}

//Scripting Functions Below

void
Player::deactivate()
{
  deactivated = true;
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
}

void
Player::activate()
{
  deactivated = false;
}

void Player::walk(float speed)
{
  physic.set_velocity_x(speed);
}

