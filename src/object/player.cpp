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
#include "scripting/squirrel_util.hpp"
#include "main.hpp"
#include "platform.hpp"
#include "badguy/badguy.hpp"
#include "player_status.hpp"
#include "log.hpp"
#include "falling_coin.hpp"
#include "random_generator.hpp"
#include "object/sprite_particle.hpp"

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
static const float CHEER_TIME = 1;

static const float UNDUCK_HURT_TIME = 0.25; /**< if Tux cannot unduck for this long, he will get hurt */

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
    arms->draw(context, pos, layer+10);
  if(feet != NULL)
    feet->draw(context, pos, layer-2);
}

Player::Player(PlayerStatus* _player_status)
  : player_status(_player_status), grabbed_object(NULL), ghost_mode(false)
{
  controller = main_controller;
  smalltux_gameover = sprite_manager->create("images/creatures/tux_small/smalltux-gameover.sprite");
  smalltux_star = sprite_manager->create("images/creatures/tux_small/smalltux-star.sprite");
  bigtux_star = sprite_manager->create("images/creatures/tux_big/bigtux-star.sprite");

  sound_manager->preload("sounds/bigjump.wav");
  sound_manager->preload("sounds/jump.wav");
  sound_manager->preload("sounds/hurt.wav");
  sound_manager->preload("sounds/skid.wav");
  sound_manager->preload("sounds/flip.wav");
  sound_manager->preload("sounds/invincible.wav");

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
    set_size(31.8, 62.8);
  else
    set_size(31.8, 30.8);

  dir = RIGHT;
  old_dir = dir;
  duck = false;
  dead = false;

  dying = false;
  peeking = AUTO;
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
  grabbed_object = NULL;

  physic.reset();
}

void
Player::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  Scripting::Player* interface = static_cast<Scripting::Player*> (this);
  Scripting::expose_object(vm, table_idx, interface, "Tux", false);
}

void
Player::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  Scripting::unexpose_object(vm, table_idx, "Tux");
}

void
Player::set_controller(Controller* controller)
{
  this->controller = controller;
}

bool
Player::adjust_height(float new_height)
{
  Rect bbox2 = bbox;
  bbox2.move(Vector(0, bbox.get_height() - new_height));
  bbox2.set_height(new_height);
  if (!Sector::current()->is_free_space(bbox2))
    return false;

  // adjust bbox accordingly
  // note that we use members of moving_object for this, so we can run this during CD, too
  set_pos(bbox2.p1);
  set_size(bbox2.get_width(), bbox2.get_height());
  return true;
}

void
Player::update(float elapsed_time)
{
  if(dying && dying_timer.check()) {
    dead = true;
    return;
  }

  if(!dying && !deactivated)
    handle_input();

  // handle_input() calls apply_friction() when Tux is not walking, so we'll have to do this ourselves
  if (deactivated) apply_friction();

  // extend/shrink tux collision rectangle so that we fall through/walk over 1
  // tile holes
  if(fabsf(physic.get_velocity_x()) > MAX_WALK_XM) {
    set_width(34);
  } else {
    set_width(31.8);
  }

  // on downward slopes, adjust vertical velocity so tux walks smoothly down
  if (on_ground()) {
    if(floor_normal.y != 0) {
      if ((floor_normal.x * physic.get_velocity_x()) >= 0) {
        physic.set_velocity_y(250);
      }
    }
  }

  // handle backflipping
  if (backflipping) {
    //prevent player from changing direction when backflipping 
    dir = (backflip_direction == 1) ? LEFT : RIGHT; 
    if (backflip_timer.started()) physic.set_velocity_x(100 * backflip_direction);
  }

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

  // check if we landed
  if(on_ground()) { 
    jumping = false;
    if (backflipping && (!backflip_timer.started())) {
      backflipping = false;
      backflip_direction = 0;

      // if controls are currently deactivated, we take care of standing up ourselves
      if (deactivated) do_standup();
    }
  }
 
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
      physic.set_velocity_y(-2);
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

  // calculate movement for this frame
  movement = physic.get_movement(elapsed_time);

  if(grabbed_object != NULL && !dying) {
    Vector pos = get_pos() + 
      Vector(dir == LEFT ? -16 : 16, get_bbox().get_height()*0.66666 - 32);
    grabbed_object->grab(*this, pos, dir);
  }
  
  if(grabbed_object != NULL && dying){
    grabbed_object->ungrab(*this, dir);
    grabbed_object = NULL;
  }

  on_ground_flag = false;

  // when invincible, spawn particles
  if (invincible_timer.started() && !dying)
  {
    if (systemRandom.rand(0, 2) == 0) {
      float px = systemRandom.randf(bbox.p1.x+0, bbox.p2.x-0);
      float py = systemRandom.randf(bbox.p1.y+0, bbox.p2.y-0);
      Vector ppos = Vector(px, py);
      Vector pspeed = Vector(0, 0);
      Vector paccel = Vector(0, 0);
      // draw bright sparkle when there is lots of time left, dark sparkle when invincibility is about to end
      if (invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING) {
	// make every other a longer sparkle to make trail a bit fuzzy
	if (size_t(game_time*20)%2) {
	  Sector::current()->add_object(new SpriteParticle("images/objects/particles/sparkle.sprite", "small", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
	} else {
	  Sector::current()->add_object(new SpriteParticle("images/objects/particles/sparkle.sprite", "medium", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
	}
      } else {
        Sector::current()->add_object(new SpriteParticle("images/objects/particles/sparkle.sprite", "dark", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
      }
    }
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
Player::apply_friction()
{
  if ((on_ground()) && (fabs(physic.get_velocity_x()) < WALK_SPEED)) {
    physic.set_velocity_x(0);
    physic.set_acceleration_x(0);
  } else if(physic.get_velocity_x() < 0) {
    physic.set_acceleration_x(WALK_ACCELERATION_X * 1.5);
  } else {
    physic.set_acceleration_x(WALK_ACCELERATION_X * -1.5);
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

  // only run if action key is pressed and we're not holding anything
  if (!(controller->hold(Controller::ACTION) && (!grabbed_object))) {
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
          Vector(dir == RIGHT ? get_bbox().p2.x : get_bbox().p1.x, get_bbox().p2.y),
          dir == RIGHT ? 270+20 : 90-40, dir == RIGHT ? 270+40 : 90-20,
          Vector(280, -260), Vector(0, 300), 3, Color(.4, .4, .4), 3, .8,
          LAYER_OBJECTS+1));
      
      ax *= 2.5;
    } else {
      ax *= 2;
    }
  }

  physic.set_velocity(vx, vy);
  physic.set_acceleration(ax, ay);

  // we get slower when not pressing any keys
  if(dirsign == 0) {
    apply_friction();
  }

}

void
Player::do_cheer()
{
  do_duck();
  do_backflip();
  do_standup();
}

void
Player::do_duck() {
  if (duck) return;
  if (!is_big()) return;

  if (physic.get_velocity_y() != 0) return;
  if (!on_ground()) return;

  if (adjust_height(31.8)) {
    duck = true;
    unduck_hurt_timer.stop();
  } else {
    // FIXME: what now?
  }
}

void 
Player::do_standup() {
  if (!duck) return;
  if (!is_big()) return;
  if (backflipping) return;

  if (adjust_height(63.8)) {
    duck = false;
    unduck_hurt_timer.stop();
  } else {
    // if timer is not already running, start it.
    if (unduck_hurt_timer.get_period() == 0) {
      unduck_hurt_timer.start(UNDUCK_HURT_TIME);
    } 
    else if (unduck_hurt_timer.check()) {
      kill(false);
    }
  }

}

void
Player::do_backflip() {
  if (!duck) return;
  if (!on_ground()) return;

  // TODO: we don't have an animation for firetux backflipping, so let's revert to bigtux
  set_bonus(GROWUP_BONUS, true);

  backflip_direction = (dir == LEFT)?(+1):(-1);
  backflipping = true;
  do_jump(-580);
  sound_manager->play("sounds/flip.wav");
  backflip_timer.start(0.15);
}

void
Player::do_jump(float yspeed) {
  if (!on_ground()) return;

  physic.set_velocity_y(yspeed);
  //bbox.move(Vector(0, -1));
  jumping = true;
  on_ground_flag = false;
  can_jump = false;

  // play sound
  if (is_big()) {
    sound_manager->play("sounds/bigjump.wav");
  } else {
    sound_manager->play("sounds/jump.wav");
  }
}

void
Player::handle_vertical_input()
{

  // Press jump key
  if(controller->pressed(Controller::JUMP) && (can_jump)) {
    if (duck) { 
      // when running, only jump a little bit; else do a backflip
      if (physic.get_velocity_x() != 0) do_jump(-300); else do_backflip();
    } else {
      // jump a bit higher if we are running; else do a normal jump
      if (fabs(physic.get_velocity_x()) > MAX_WALK_XM) do_jump(-580); else do_jump(-520);
    }
  } 
  // Let go of jump key
  else if(!controller->hold(Controller::JUMP)) { 
    if (!backflipping && jumping && physic.get_velocity_y() < 0) {
      jumping = false;
      physic.set_velocity_y(0);
    }
  }

  /* In case the player has pressed Down while in a certain range of air,
     enable butt jump action */
  if (controller->hold(Controller::DOWN) && !butt_jump && !duck && is_big() && jumping) {
    butt_jump = true;
  }
  
  /* When Down is not held anymore, disable butt jump */
  if(butt_jump && !controller->hold(Controller::DOWN))
    butt_jump = false;
}

void
Player::handle_input()
{
  if (ghost_mode) {
    handle_input_ghost();
    return;
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
      grabbed_object = NULL;
    }
  }

  /* Peeking */
  if( controller->released( Controller::PEEK_LEFT ) ) {
    peeking = AUTO;
  } 
  if( controller->released( Controller::PEEK_RIGHT ) ) {
    peeking = AUTO;
  }
  if( controller->pressed( Controller::PEEK_LEFT ) ) {
    peeking = LEFT;
  } 
  if( controller->pressed( Controller::PEEK_RIGHT ) ) {
    peeking = RIGHT;
  }
 
  /* Handle horizontal movement: */
  if (!backflipping) handle_horizontal_input();
  
  /* Jump/jumping? */
  if (on_ground() && !controller->hold(Controller::JUMP))
    can_jump = true;

  /* Handle vertical movement: */
  handle_vertical_input();

  /* Shoot! */
  if (controller->pressed(Controller::ACTION) && player_status->bonus == FIRE_BONUS) {
    if(Sector::current()->add_bullet(
         get_pos() + ((dir == LEFT)? Vector(0, bbox.get_height()/2) 
                      : Vector(32, bbox.get_height()/2)),
         physic.get_velocity_x(), dir))
      shooting_timer.start(SHOOTING_TIME);
  }
  
  /* Duck or Standup! */
  if (controller->hold(Controller::DOWN)) do_duck(); else do_standup();

}

void
Player::handle_input_ghost()
{
  float vx = 0;
  float vy = 0;
  if (controller->hold(Controller::LEFT)) { 
    dir = LEFT; 
    vx -= MAX_RUN_XM * 2; 
  }
  if (controller->hold(Controller::RIGHT)) { 
    dir = RIGHT; 
    vx += MAX_RUN_XM * 2; 
  }
  if ((controller->hold(Controller::UP)) || (controller->hold(Controller::JUMP))) {
    vy -= MAX_RUN_XM * 2;
  }
  if (controller->hold(Controller::DOWN)) {
    vy += MAX_RUN_XM * 2;
  }
  if (controller->hold(Controller::ACTION)) {
    set_ghost_mode(false);
  }
  physic.set_velocity(vx, vy);
  physic.set_acceleration(0, 0);
}

void
Player::add_coins(int count)
{
  player_status->add_coins(count);
}

bool
Player::add_bonus(const std::string& bonustype)
{
  BonusType type = NO_BONUS;
  
  if(bonustype == "grow") {
    type = GROWUP_BONUS;
  } else if(bonustype == "fireflower") {
    type = FIRE_BONUS;
  } else if(bonustype == "iceflower") {
    type = ICE_BONUS;
  } else if(bonustype == "none") {
    type = NO_BONUS;
  } else {
    std::ostringstream msg;
    msg << "Unknown bonus type "  << bonustype;
    throw std::runtime_error(msg.str());
  }
  
  return add_bonus(type);
}

bool
Player::add_bonus(BonusType type, bool animate)
{
  // always ignore NO_BONUS
  if (type == NO_BONUS) {
    return true;
  }

  // ignore GROWUP_BONUS if we're already big
  if (type == GROWUP_BONUS) {
    if (player_status->bonus == GROWUP_BONUS)
      return true; 
    if (player_status->bonus == FIRE_BONUS)
      return true;
    if (player_status->bonus == ICE_BONUS)
      return true;
  }

  return set_bonus(type, animate);
}

bool
Player::set_bonus(BonusType type, bool animate)
{
  if(player_status->bonus == NO_BONUS) {
    if (!adjust_height(62.8)) {
      printf("can't adjust\n");
      return false;
    }
    if(animate)
      growing_timer.start(GROWING_TIME);
  }

  if ((type == NO_BONUS) || (type == GROWUP_BONUS)) {
    if ((player_status->bonus == FIRE_BONUS) && (animate)) {
      // visually lose helmet
      Vector ppos = Vector((bbox.p1.x + bbox.p2.x) / 2, bbox.p1.y);
      Vector pspeed = Vector(((dir==LEFT) ? +100 : -100), -300);
      Vector paccel = Vector(0, 1000);
      std::string action = (dir==LEFT)?"left":"right";
      Sector::current()->add_object(new SpriteParticle("images/objects/particles/firetux-helmet.sprite", action, ppos, ANCHOR_TOP, pspeed, paccel, LAYER_OBJECTS-1));
    }
    player_status->max_fire_bullets = 0;
    player_status->max_ice_bullets = 0;
  }
  if (type == FIRE_BONUS) player_status->max_fire_bullets++;
  if (type == ICE_BONUS) player_status->max_ice_bullets++;

  player_status->bonus = type;
  return true;
}

void
Player::set_visible(bool visible)
{
  this->visible = visible;
  if( visible ) 
    set_group(COLGROUP_MOVING);
  else
    set_group(COLGROUP_DISABLED);
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

  int layer = LAYER_OBJECTS + 1;

  /* Set Tux sprite action */
  if (backflipping)
    {
    if(dir == LEFT)
      tux_body->set_action("backflip-left");
    else // dir == RIGHT
      tux_body->set_action("backflip-right");
    }
  else if (duck && is_big())
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
    smalltux_gameover->draw(context, get_pos(), LAYER_FLOATINGOBJECTS + 1);
  } 
  else if ((growing_timer.get_timeleft() > 0) && (!duck)) {
      if (dir == RIGHT) {
        context.draw_surface(growingtux_right[int((growing_timer.get_timegone() *
                 GROWING_FRAMES) / GROWING_TIME)], get_pos(), layer);
      } else {
        context.draw_surface(growingtux_left[int((growing_timer.get_timegone() *
                GROWING_FRAMES) / GROWING_TIME)], get_pos(), layer);
      }
    }
  else if (safe_timer.started() && size_t(game_time*40)%2)
    ;  // don't draw Tux
  else
    tux_body->draw(context, get_pos(), layer);

}

void
Player::collision_tile(uint32_t tile_attributes)
{
  if(tile_attributes & Tile::HURTS)
    kill(false);
}

void
Player::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom) {
    if(physic.get_velocity_y() > 0)
      physic.set_velocity_y(0);

    on_ground_flag = true;
    floor_normal = hit.slope_normal;
  } else if(hit.top) {
    if(physic.get_velocity_y() < 0)
      physic.set_velocity_y(.2);
  }

  if(hit.left || hit.right) {
    physic.set_velocity_x(0);
  }

  // crushed?
  if(hit.crush) {
    if(hit.left || hit.right) {
      kill(true);
    } else if(hit.top || hit.bottom) {
      kill(false);
    }
  }
}

HitResponse
Player::collision(GameObject& other, const CollisionHit& hit)
{
  Bullet* bullet = dynamic_cast<Bullet*> (&other);
  if(bullet) {
    return FORCE_MOVE;
  }

  // if we hit something from the side that is portable, the ACTION button is pressed and we are not already holding anything: grab it
  if ((hit.left || hit.right) && (other.get_flags() & FLAG_PORTABLE) && controller->hold(Controller::ACTION) && (!grabbed_object)) {
    Portable* portable = dynamic_cast<Portable*> (&other);
    assert(portable != NULL);
    if(portable) {
      grabbed_object = portable;
      grabbed_object->grab(*this, get_pos(), dir);
      return CONTINUE;
    }
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
    if(safe_timer.started() || invincible_timer.started())
      return FORCE_MOVE;

    return CONTINUE;
  }

  return CONTINUE;
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
Player::kill(bool completely)
{
  if(dying || deactivated)
    return;

  if(!completely && (safe_timer.started() || invincible_timer.started()))
    return;                          
  
  sound_manager->play("sounds/hurt.wav");

  physic.set_velocity_x(0);

  if(!completely && is_big()) {
    if(player_status->bonus == FIRE_BONUS
        || player_status->bonus == ICE_BONUS) {
      safe_timer.start(TUX_SAFE_TIME);
      set_bonus(GROWUP_BONUS, true);
    } else {
      //growing_timer.start(GROWING_TIME);
      safe_timer.start(TUX_SAFE_TIME /* + GROWING_TIME */);
      adjust_height(30.8);
      duck = false;
      set_bonus(NO_BONUS, true);
    }
  } else {
    for (int i = 0; (i < 5) && (i < player_status->coins); i++)
    {
      // the numbers: starting x, starting y, velocity y
      Sector::current()->add_object(new FallingCoin(get_pos() + 
            Vector(systemRandom.rand(5), systemRandom.rand(-32,18)), 
            systemRandom.rand(-100,100)));
    }
    physic.enable_gravity(true);
    physic.set_acceleration(0, 0);
    physic.set_velocity(0, -700);
    player_status->coins -= 25;
    set_bonus(NO_BONUS, true);
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
  set_pos(vector);

  // TODO: do we need the following? Seems irrelevant to moving the player
  if(is_big())
    set_size(31.8, 63.8);
  else
    set_size(31.8, 31.8);
  duck = false;
  last_ground_y = vector.y;

  physic.reset();
}

void
Player::check_bounds(Camera* camera)
{
  /* Keep tux in bounds: */
  if (get_pos().x < 0) {
    // Lock Tux to the size of the level, so that he doesn't fall of
    // on the left side
    set_pos(Vector(0, get_pos().y));
  }

  /* Keep in-bounds, vertically: */
  if (get_pos().y > Sector::current()->solids->get_height() * 32) {
    kill(true);
    return;
  }

  bool adjust = false;
  // can happen if back scrolling is disabled
  if(get_pos().x < camera->get_translation().x) {
    set_pos(Vector(camera->get_translation().x, get_pos().y));
    adjust = true;
  }
  if(get_pos().x >= camera->get_translation().x + SCREEN_WIDTH - bbox.get_width())
  {
    set_pos(Vector(
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
Player::add_velocity(const Vector& velocity, const Vector& end_speed)
{
  if (end_speed.x > 0) physic.set_velocity_x(std::min(physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.x < 0) physic.set_velocity_x(std::max(physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.y > 0) physic.set_velocity_y(std::min(physic.get_velocity_y() + velocity.y, end_speed.y));
  if (end_speed.y < 0) physic.set_velocity_y(std::max(physic.get_velocity_y() + velocity.y, end_speed.y));
}

void
Player::bounce(BadGuy& )
{
  if(controller->hold(Controller::JUMP))
    physic.set_velocity_y(-520);
  else
    physic.set_velocity_y(-300);
}

//Scripting Functions Below

void
Player::deactivate()
{
  if (deactivated) return;
  deactivated = true;
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  physic.set_acceleration_x(0);
  physic.set_acceleration_y(0);
}

void
Player::activate()
{
  if (!deactivated) return;
  deactivated = false;
}

void Player::walk(float speed)
{
  physic.set_velocity_x(speed);
}

void
Player::set_ghost_mode(bool enable)
{
  if (ghost_mode == enable) return;
  if (enable) {
    ghost_mode = true;
    set_group(COLGROUP_DISABLED);
    physic.enable_gravity(false);
    log_debug << "You feel lightheaded. Use movement controls to float around, press ACTION to scare badguys." << std::endl;
  } else {
    ghost_mode = false;
    set_group(COLGROUP_MOVING);
    physic.enable_gravity(true);
    log_debug << "You feel solid again." << std::endl;
  }
}

