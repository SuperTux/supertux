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

#include "object/player.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "math/random_generator.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/falling_coin.hpp"
#include "object/particles.hpp"
#include "object/portable.hpp"
#include "object/sprite_particle.hpp"
#include "scripting/player.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "trigger/climbable.hpp"

#include <math.h>

//#define SWIMMING

namespace {
static const float BUTTJUMP_MIN_VELOCITY_Y = 400.0f;
static const float SHOOTING_TIME = .150f;
static const float GLIDE_TIME_PER_FLOWER = 0.5f;
static const float STONE_TIME_PER_FLOWER = 2.0f;

/** number of idle stages, including standing */
static const unsigned int IDLE_STAGE_COUNT = 5;
/**
 * how long to play each idle animation in milliseconds
 * '0' means the sprite action is played once before moving onto the next
 * animation
 */
static const int IDLE_TIME[] = { 5000, 0, 2500, 0, 2500 };
/** idle stages */
static const std::string IDLE_STAGES[] =
{ "stand",
  "idle",
  "stand",
  "idle",
  "stand" };

/** acceleration in horizontal direction when walking
 * (all accelerations are in  pixel/s^2) */
static const float WALK_ACCELERATION_X = 300;
/** acceleration in horizontal direction when running */
static const float RUN_ACCELERATION_X = 400;
/** acceleration when skidding */
static const float SKID_XM = 200;
/** time of skidding in seconds */
static const float SKID_TIME = .3f;
/** maximum walk velocity (pixel/s) */
static const float MAX_WALK_XM = 230;
/** maximum run velocity (pixel/s) */
static const float MAX_RUN_XM = 320;
/** bonus run velocity addition (pixel/s) */
static const float BONUS_RUN_XM = 80;
/** maximum horizontal climb velocity */
static const float MAX_CLIMB_XM = 96;
/** maximum vertical climb velocity */
static const float MAX_CLIMB_YM = 128;
/** maximum vertical glide velocity */
static const float MAX_GLIDE_YM = 128;
/** instant velocity when tux starts to walk */
static const float WALK_SPEED = 100;

/** multiplied by WALK_ACCELERATION to give friction */
static const float NORMAL_FRICTION_MULTIPLIER = 1.5f;
/** multiplied by WALK_ACCELERATION to give friction */
static const float ICE_FRICTION_MULTIPLIER = 0.1f;
static const float ICE_ACCELERATION_MULTIPLIER = 0.25f;

/** time of the kick (kicking mriceblock) animation */
static const float KICK_TIME = .3f;

/** if Tux cannot unduck for this long, he will get hurt */
static const float UNDUCK_HURT_TIME = 0.25f;
/** gravity is higher after the jump key is released before
    the apex of the jump is reached */
static const float JUMP_EARLY_APEX_FACTOR = 3.0;

static const float JUMP_GRACE_TIME = 0.25f; /**< time before hitting the ground that the jump button may be pressed (and still trigger a jump) */

/* Tux's collision rectangle */
static const float TUX_WIDTH = 31.8f;
static const float RUNNING_TUX_WIDTH = 34;
static const float SMALL_TUX_HEIGHT = 30.8f;
static const float BIG_TUX_HEIGHT = 62.8f;
static const float DUCKED_TUX_HEIGHT = 31.8f;

bool no_water = true;
}

Player::Player(PlayerStatus* _player_status, const std::string& name_) :
  deactivated(false),
  controller(),
  scripting_controller(),
  player_status(_player_status),
  duck(false),
  dead(false),
  dying(false),
  winning(false),
  backflipping(false),
  backflip_direction(0),
  peekingX(AUTO),
  peekingY(AUTO),
  ability_time(),
  stone(false),
  swimming(false),
  speedlimit(0), //no special limit
  scripting_controller_old(0),
  jump_early_apex(false),
  on_ice(false),
  ice_this_frame(false),
  lightsprite(SpriteManager::current()->create("images/creatures/tux/light.sprite")),
  powersprite(SpriteManager::current()->create("images/creatures/tux/powerups.sprite")),
  dir(RIGHT),
  old_dir(dir),
  last_ground_y(0),
  fall_mode(ON_GROUND),
  on_ground_flag(false),
  jumping(false),
  can_jump(true),
  jump_button_timer(),
  wants_buttjump(false),
  does_buttjump(false),
  invincible_timer(),
  skidding_timer(),
  safe_timer(),
  kick_timer(),
  shooting_timer(),
  ability_timer(),
  cooldown_timer(),
  dying_timer(),
  growing(false),
  backflip_timer(),
  physic(),
  visible(true),
  grabbed_object(NULL),
  sprite(),
  airarrow(),
  floor_normal(),
  ghost_mode(false),
  edit_mode(false),
  unduck_hurt_timer(),
  idle_timer(),
  idle_stage(0),
  climbing(0)
{
  this->name = name_;
  controller = InputManager::current()->get_controller();
  scripting_controller.reset(new CodeController());
  // if/when we have complete penny gfx, we can
  // load those instead of Tux's sprite in the
  // constructor
  sprite = SpriteManager::current()->create("images/creatures/tux/tux.sprite");
  airarrow = Surface::create("images/engine/hud/airarrow.png");
  idle_timer.start(IDLE_TIME[0]/1000.0f);

  SoundManager::current()->preload("sounds/bigjump.wav");
  SoundManager::current()->preload("sounds/jump.wav");
  SoundManager::current()->preload("sounds/hurt.wav");
  SoundManager::current()->preload("sounds/kill.wav");
  SoundManager::current()->preload("sounds/skid.wav");
  SoundManager::current()->preload("sounds/flip.wav");
  SoundManager::current()->preload("sounds/invincible_start.ogg");
  SoundManager::current()->preload("sounds/splash.wav");

  if(is_big())
    set_size(TUX_WIDTH, BIG_TUX_HEIGHT);
  else
    set_size(TUX_WIDTH, SMALL_TUX_HEIGHT);

  sprite->set_angle(0.0f);
  powersprite->set_angle(0.0f);
  lightsprite->set_angle(0.0f);
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));

  physic.reset();
}

Player::~Player()
{
  if (climbing) stop_climbing(*climbing);
}

void
Player::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  auto obj = new scripting::Player(this);
  scripting::expose_object(vm, table_idx, obj, name, false);
}

void
Player::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  scripting::unexpose_object(vm, table_idx, name);
}

float
Player::get_speedlimit() const
{
  return speedlimit;
}

void
Player::set_speedlimit(float newlimit)
{
  speedlimit=newlimit;
}

void
Player::set_controller(Controller* controller_)
{
  this->controller = controller_;
}

void
Player::set_winning()
{
  if( ! is_winning() ){
    winning = true;
    invincible_timer.start(10000.0f);
  }
}

void
Player::use_scripting_controller(bool use_or_release)
{
  if ((use_or_release == true) && (controller != scripting_controller.get())) {
    scripting_controller_old = get_controller();
    set_controller(scripting_controller.get());
  }
  if ((use_or_release == false) && (controller == scripting_controller.get())) {
    set_controller(scripting_controller_old);
    scripting_controller_old = 0;
  }
}

void
Player::do_scripting_controller(const std::string& control, bool pressed)
{
  for(int i = 0; Controller::controlNames[i] != 0; ++i) {
    if(control == std::string(Controller::controlNames[i])) {
      scripting_controller->press(Controller::Control(i), pressed);
    }
  }
}

bool
Player::adjust_height(float new_height)
{
  Rectf bbox2 = bbox;
  bbox2.move(Vector(0, bbox.get_height() - new_height));
  bbox2.set_height(new_height);


  if(new_height > bbox.get_height()) {
    Rectf additional_space = bbox2;
    additional_space.set_height(new_height - bbox.get_height());
    if(!Sector::current()->is_free_of_statics(additional_space, this, true))
      return false;
  }

  // adjust bbox accordingly
  // note that we use members of moving_object for this, so we can run this during CD, too
  set_pos(bbox2.p1);
  set_size(bbox2.get_width(), bbox2.get_height());
  return true;
}

void
Player::trigger_sequence(const std::string& sequence_name)
{
  trigger_sequence(string_to_sequence(sequence_name));
}

void
Player::trigger_sequence(Sequence seq)
{
  if (climbing) stop_climbing(*climbing);
  backflipping = false;
  backflip_direction = 0;
  sprite->set_angle(0.0f);
  powersprite->set_angle(0.0f);
  lightsprite->set_angle(0.0f);
  GameSession::current()->start_sequence(seq);
}

void
Player::update(float elapsed_time)
{
  if( no_water ){
    swimming = false;
  }
  no_water = true;

  if(dying && dying_timer.check()) {
    set_bonus(NO_BONUS, true);
    dead = true;
    return;
  }

  if(!dying && !deactivated)
    handle_input();

  /*
  // handle_input() calls apply_friction() when Tux is not walking, so we'll have to do this ourselves
  if (deactivated)
  apply_friction();
  */

  // extend/shrink tux collision rectangle so that we fall through/walk over 1
  // tile holes
  if(fabsf(physic.get_velocity_x()) > MAX_WALK_XM) {
    set_width(RUNNING_TUX_WIDTH);
  } else {
    set_width(TUX_WIDTH);
  }

  // on downward slopes, adjust vertical velocity so tux walks smoothly down
  if (on_ground() && !dying) {
    if(floor_normal.y != 0) {
      if ((floor_normal.x * physic.get_velocity_x()) >= 0) {
        physic.set_velocity_y(250);
      }
    }
  }

  // handle backflipping
  if (backflipping && !dying) {
    //prevent player from changing direction when backflipping
    dir = (backflip_direction == 1) ? LEFT : RIGHT;
    if (backflip_timer.started()) physic.set_velocity_x(100 * backflip_direction);
    //rotate sprite during flip
    sprite->set_angle(sprite->get_angle() + (dir==LEFT?1:-1) * elapsed_time * (360.0f / 0.5f));
    if (player_status->bonus == EARTH_BONUS || player_status->bonus == AIR_BONUS ||
        (player_status->bonus == FIRE_BONUS && g_config->christmas_mode)) {
      powersprite->set_angle(sprite->get_angle());
      if (player_status->bonus == EARTH_BONUS)
        lightsprite->set_angle(sprite->get_angle());
    }
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
    if (backflipping && (backflip_timer.get_timegone() > 0.15f)) {
      backflipping = false;
      backflip_direction = 0;
      physic.set_velocity_x(0);
      if (!stone) {
        sprite->set_angle(0.0f);
        powersprite->set_angle(0.0f);
        lightsprite->set_angle(0.0f);
      }

      // if controls are currently deactivated, we take care of standing up ourselves
      if (deactivated)
        do_standup();
    }
    if (player_status->bonus == AIR_BONUS)
      ability_time = player_status->max_air_time * GLIDE_TIME_PER_FLOWER;
  }

  // calculate movement for this frame
  movement = physic.get_movement(elapsed_time);

  if(grabbed_object != NULL && !dying) {
    position_grabbed_object();
  }

  if(grabbed_object != NULL && dying){
    grabbed_object->ungrab(*this, dir);
    grabbed_object = NULL;
  }

  if(!ice_this_frame && on_ground())
    on_ice = false;

  on_ground_flag = false;
  ice_this_frame = false;

  // when invincible, spawn particles
  if (invincible_timer.started())
  {
    if (graphicsRandom.rand(0, 2) == 0) {
      float px = graphicsRandom.randf(bbox.p1.x+0, bbox.p2.x-0);
      float py = graphicsRandom.randf(bbox.p1.y+0, bbox.p2.y-0);
      Vector ppos = Vector(px, py);
      Vector pspeed = Vector(0, 0);
      Vector paccel = Vector(0, 0);
      Sector::current()->add_object(std::make_shared<SpriteParticle>(
                                      "images/objects/particles/sparkle.sprite",
                                      // draw bright sparkle when there is lots of time left,
                                      // dark sparkle when invincibility is about to end
                                      (invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING) ?
                                      // make every other a longer sparkle to make trail a bit fuzzy
                                      (size_t(game_time*20)%2) ? "small" : "medium"
                                      :
                                      "dark", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
    }
  }

  if (growing) {
    if (sprite->animation_done()) growing = false;
  }

  // when climbing animate only while moving
  if(climbing){
    if((physic.get_velocity_x()==0)&&(physic.get_velocity_y()==0))
      sprite->stop_animation();
    else
      sprite->set_animation_loops(-1);
  }

}

bool
Player::slightly_above_ground() const
{
  float abs_vy = std::abs(physic.get_velocity_y());
  float ground_y_delta = std::abs(last_ground_y - get_pos().y);
  return (abs_vy == 15.625 || abs_vy == 31.25) && ground_y_delta < 0.85;
}

bool
Player::on_ground() const
{
  return on_ground_flag || slightly_above_ground();
}

bool
Player::is_big() const
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
  } else {
    float friction = WALK_ACCELERATION_X * (on_ice ? ICE_FRICTION_MULTIPLIER : NORMAL_FRICTION_MULTIPLIER);
    if(physic.get_velocity_x() < 0) {
      physic.set_acceleration_x(friction);
    } else if(physic.get_velocity_x() > 0) {
      physic.set_acceleration_x(-friction);
    } // no friction for physic.get_velocity_x() == 0
  }
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

  // do not run if we're holding something which slows us down
  if ( grabbed_object && grabbed_object->is_hampering() ) {
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
    if( vx * dirsign < MAX_WALK_XM ) {
      ax = dirsign * WALK_ACCELERATION_X;
    } else {
      ax = dirsign * RUN_ACCELERATION_X;
    }
    // limit speed
    if(vx >= MAX_RUN_XM + BONUS_RUN_XM *((player_status->bonus == AIR_BONUS) ? 1 : 0) && dirsign > 0) {
      vx = MAX_RUN_XM + BONUS_RUN_XM *((player_status->bonus == AIR_BONUS) ? 1 : 0);
      ax = 0;
    } else if(vx <= -MAX_RUN_XM - BONUS_RUN_XM *((player_status->bonus == AIR_BONUS) ? 1 : 0) && dirsign < 0) {
      vx = -MAX_RUN_XM - BONUS_RUN_XM *((player_status->bonus == AIR_BONUS) ? 1 : 0);
      ax = 0;
    }
  }

  // we can reach WALK_SPEED without any acceleration
  if(dirsign != 0 && fabs(vx) < WALK_SPEED) {
    vx = dirsign * WALK_SPEED;
  }

  //Check speedlimit.
  if( speedlimit > 0 &&  vx * dirsign >= speedlimit ) {
    vx = dirsign * speedlimit;
    ax = 0;
  }

  // changing directions?
  if(on_ground() && ((vx < 0 && dirsign >0) || (vx>0 && dirsign<0))) {
    // let's skid!
    if(fabs(vx)>SKID_XM && !skidding_timer.started()) {
      skidding_timer.start(SKID_TIME);
      SoundManager::current()->play("sounds/skid.wav");
      // dust some particles
      Sector::current()->add_object(
        std::make_shared<Particles>(
          Vector(dir == LEFT ? bbox.p2.x : bbox.p1.x, bbox.p2.y),
          dir == LEFT ? 50 : -70, dir == LEFT ? 70 : -50, 260, 280,
          Vector(0, 300), 3, Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1));

      ax *= 2.5;
    } else {
      ax *= 2;
    }
  }

  if(on_ice) {
    ax *= ICE_ACCELERATION_MULTIPLIER;
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
  if (duck)
    return;
  if (!is_big())
    return;

  if (physic.get_velocity_y() != 0)
    return;
  if (!on_ground())
    return;
  if (does_buttjump)
    return;

  if (adjust_height(DUCKED_TUX_HEIGHT)) {
    duck = true;
    growing = false;
    unduck_hurt_timer.stop();
  } else {
    // FIXME: what now?
  }
}

void
Player::do_standup() {
  if (!duck)
    return;
  if (!is_big())
    return;
  if (backflipping)
    return;
  if (stone)
    return;

  if (adjust_height(BIG_TUX_HEIGHT)) {
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
  if (!duck)
    return;
  if (!on_ground())
    return;

  backflip_direction = (dir == LEFT)?(+1):(-1);
  backflipping = true;
  do_jump((player_status->bonus == AIR_BONUS) ? -720 : -580);
  SoundManager::current()->play("sounds/flip.wav");
  backflip_timer.start(TUX_BACKFLIP_TIME);
}

void
Player::do_jump(float yspeed) {
  if (!on_ground())
    return;

  physic.set_velocity_y(yspeed);
  //bbox.move(Vector(0, -1));
  jumping = true;
  on_ground_flag = false;
  can_jump = false;

  // play sound
  if (is_big()) {
    SoundManager::current()->play("sounds/bigjump.wav");
  } else {
    SoundManager::current()->play("sounds/jump.wav");
  }
}

void
Player::early_jump_apex()
{
  if (!jump_early_apex)
  {
    jump_early_apex = true;
    physic.set_gravity_modifier(JUMP_EARLY_APEX_FACTOR);
  }
}

void
Player::do_jump_apex()
{
  if (jump_early_apex)
  {
    jump_early_apex = false;
    physic.set_gravity_modifier(1.0f);
  }
}

void
Player::handle_vertical_input()
{
  // Press jump key
  if(controller->pressed(Controller::JUMP)) jump_button_timer.start(JUMP_GRACE_TIME);
  if(controller->hold(Controller::JUMP) && jump_button_timer.started() && can_jump) {
    jump_button_timer.stop();
    if (duck) {
      // when running, only jump a little bit; else do a backflip
      if ((physic.get_velocity_x() != 0) ||
          (controller->hold(Controller::LEFT)) ||
          (controller->hold(Controller::RIGHT)))
      {
        do_jump(-300);
      }
      else
      {
        do_backflip();
      }
    } else {
      // airflower allows for higher jumps-
      // jump a bit higher if we are running; else do a normal jump
      if(player_status->bonus == AIR_BONUS)
        do_jump((fabs(physic.get_velocity_x()) > MAX_WALK_XM) ? -620 : -580);
      else
        do_jump((fabs(physic.get_velocity_x()) > MAX_WALK_XM) ? -580 : -520);
    }
    // airflower glide only when holding jump key
  } else  if (controller->hold(Controller::JUMP) && player_status->bonus == AIR_BONUS && physic.get_velocity_y() > MAX_GLIDE_YM) {
      if (ability_time > 0 && !ability_timer.started())
        ability_timer.start(ability_time);
      else if (ability_timer.started()) {
        // glide stops after some duration or if buttjump is initiated
        if ((ability_timer.get_timeleft() <= 0.05f) || controller->hold(Controller::DOWN)) {
          ability_time = 0;
          ability_timer.stop();
        } else {
          physic.set_velocity_y(MAX_GLIDE_YM);
          physic.set_acceleration_y(0);
        }
      }
    }


  // Let go of jump key
  else if(!controller->hold(Controller::JUMP)) {
    if (!backflipping && jumping && physic.get_velocity_y() < 0) {
      jumping = false;
      early_jump_apex();
    }
    if (player_status->bonus == AIR_BONUS && ability_timer.started()){
      ability_time = ability_timer.get_timeleft();
      ability_timer.stop();
    }
  }

  if(jump_early_apex && physic.get_velocity_y() >= 0) {
    do_jump_apex();
  }

  /* In case the player has pressed Down while in a certain range of air,
     enable butt jump action */
  if (controller->hold(Controller::DOWN) && !duck && is_big() && !on_ground()) {
    wants_buttjump = true;
    if (physic.get_velocity_y() >= BUTTJUMP_MIN_VELOCITY_Y) does_buttjump = true;
  }

  /* When Down is not held anymore, disable butt jump */
  if(!controller->hold(Controller::DOWN)) {
    wants_buttjump = false;
    does_buttjump = false;
  }

  // swimming
  physic.set_acceleration_y(0);
#ifdef SWIMMING
  if (swimming) {
    if (controller->hold(Controller::UP) || controller->hold(Controller::JUMP))
      physic.set_acceleration_y(-2000);
    physic.set_velocity_y(physic.get_velocity_y() * 0.94);
  }
#endif
}

void
Player::handle_input()
{
  if (ghost_mode) {
    handle_input_ghost();
    return;
  }
  if (climbing) {
    handle_input_climbing();
    return;
  }

  /* Peeking */
  if( controller->released( Controller::PEEK_LEFT ) || controller->released( Controller::PEEK_RIGHT ) ) {
    peekingX = AUTO;
  }
  if( controller->released( Controller::PEEK_UP ) || controller->released( Controller::PEEK_DOWN ) ) {
    peekingY = AUTO;
  }
  if( controller->pressed( Controller::PEEK_LEFT ) ) {
    peekingX = LEFT;
  }
  if( controller->pressed( Controller::PEEK_RIGHT ) ) {
    peekingX = RIGHT;
  }
  if(!backflipping && !jumping && on_ground()) {
    if( controller->pressed( Controller::PEEK_UP ) ) {
      peekingY = UP;
    } else if( controller->pressed( Controller::PEEK_DOWN ) ) {
      peekingY = DOWN;
    }
  }

  /* Handle horizontal movement: */
  if (!backflipping && !stone) handle_horizontal_input();

  /* Jump/jumping? */
  if (on_ground())
    can_jump = true;

  /* Handle vertical movement: */
  if (!stone) handle_vertical_input();

  /* Shoot! */
  if (controller->pressed(Controller::ACTION) && (player_status->bonus == FIRE_BONUS || player_status->bonus == ICE_BONUS)) {
    if((player_status->bonus == FIRE_BONUS &&
      Sector::current()->get_active_bullets() < player_status->max_fire_bullets) ||
      (player_status->bonus == ICE_BONUS &&
      Sector::current()->get_active_bullets() < player_status->max_ice_bullets))
    {
      Vector pos = get_pos() + ((dir == LEFT)? Vector(0, bbox.get_height()/2) : Vector(32, bbox.get_height()/2));
      auto new_bullet = std::make_shared<Bullet>(pos, physic.get_velocity_x(), dir, player_status->bonus);
      Sector::current()->add_object(new_bullet);

      SoundManager::current()->play("sounds/shoot.wav");
      shooting_timer.start(SHOOTING_TIME);
    }
  }

  /* Turn to Stone */
  if (controller->pressed(Controller::DOWN) && player_status->bonus == EARTH_BONUS && !cooldown_timer.started()) {
    if (controller->hold(Controller::ACTION) && !ability_timer.started()) {
      ability_timer.start(player_status->max_earth_time * STONE_TIME_PER_FLOWER);
      powersprite->stop_animation();
      stone = true;
      physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
    }
  }

  if (stone)
    apply_friction();

  /* Revert from Stone */
  if (stone && (!controller->hold(Controller::ACTION) || ability_timer.get_timeleft() <= 0.5f)) {
    cooldown_timer.start(ability_timer.get_timegone()/2.0f); //The longer stone form is used, the longer until it can be used again
    ability_timer.stop();
    sprite->set_angle(0.0f);
    powersprite->set_angle(0.0f);
    lightsprite->set_angle(0.0f);
    stone = false;
    for (int i = 0; i < 8; i++)
    {
      Vector ppos = Vector(bbox.get_left() + 8 + 16*((int)i/4), bbox.get_top() + 16*(i%4));
      float grey = graphicsRandom.randf(.4f, .8f);
      Color pcolor = Color(grey, grey, grey);
      Sector::current()->add_object(std::make_shared<Particles>(ppos, -60, 240, 42, 81, Vector(0, 500),
                                                                8, pcolor, 4 + graphicsRandom.randf(-0.4, 0.4),
                                                                0.8 + graphicsRandom.randf(0, 0.4), LAYER_OBJECTS+2));
    }
  }

  /* Duck or Standup! */
  if (controller->hold(Controller::DOWN) && !stone) {
    do_duck();
  } else {
    do_standup();
  }

  /* grabbing */
  try_grab();

  if(!controller->hold(Controller::ACTION) && grabbed_object) {
    auto moving_object = dynamic_cast<MovingObject*> (grabbed_object);
    auto sector = Sector::current();
    if(moving_object) {
      // move the grabbed object a bit away from tux
      Rectf grabbed_bbox = moving_object->get_bbox();
      Rectf dest_;
      dest_.p2.y = bbox.get_top() + bbox.get_height()*0.66666;
      dest_.p1.y = dest_.p2.y - grabbed_bbox.get_height();
      if(dir == LEFT) {
        dest_.p2.x = bbox.get_left() - 1;
        dest_.p1.x = dest_.p2.x - grabbed_bbox.get_width();
      } else {
        dest_.p1.x = bbox.get_right() + 1;
        dest_.p2.x = dest_.p1.x + grabbed_bbox.get_width();
      }
      if(sector->is_free_of_tiles(dest_, true) &&
         sector->is_free_of_statics(dest_, moving_object, true)) {
        moving_object->set_pos(dest_.p1);
        if(controller->hold(Controller::UP)) {
          grabbed_object->ungrab(*this, UP);
        } else {
          grabbed_object->ungrab(*this, dir);
        }
        grabbed_object = NULL;
      }
    } else {
      log_debug << "Non MovingObject grabbed?!?" << std::endl;
    }
  }

  /* stop backflipping at will */
  if( backflipping && ( !controller->hold(Controller::JUMP) && !backflip_timer.started()) ){
    backflipping = false;
    backflip_direction = 0;
    sprite->set_angle(0.0f);
    powersprite->set_angle(0.0f);
    lightsprite->set_angle(0.0f);
  }
}

void
Player::position_grabbed_object()
{
  auto moving_object = dynamic_cast<MovingObject*>(grabbed_object);
  assert(moving_object);
  auto object_bbox = moving_object->get_bbox();

  // Position where we will hold the lower-inner corner
  Vector pos(bbox.get_left() + bbox.get_width()/2,
      bbox.get_top() + bbox.get_height()*0.66666);

  // Adjust to find the grabbed object's upper-left corner
  if (dir == LEFT)
    pos.x -= object_bbox.get_width();
  pos.y -= object_bbox.get_height();

  grabbed_object->grab(*this, pos, dir);
}

void
Player::try_grab()
{
  if(controller->hold(Controller::ACTION) && !grabbed_object
     && !duck) {
    Sector* sector = Sector::current();
    Vector pos;
    if(dir == LEFT) {
      pos = Vector(bbox.get_left() - 5, bbox.get_bottom() - 16);
    } else {
      pos = Vector(bbox.get_right() + 5, bbox.get_bottom() - 16);
    }

    for(Sector::Portables::iterator i = sector->portables.begin();
        i != sector->portables.end(); ++i) {
      Portable* portable = *i;
      if(!portable->is_portable())
        continue;

      // make sure the Portable is a MovingObject
      MovingObject* moving_object = dynamic_cast<MovingObject*> (portable);
      assert(moving_object);

      // make sure the Portable isn't currently non-solid
      if(moving_object->get_group() == COLGROUP_DISABLED) continue;

      // check if we are within reach
      if(moving_object->get_bbox().contains(pos)) {
        if (climbing) stop_climbing(*climbing);
        grabbed_object = portable;
        position_grabbed_object();
        break;
      }
    }
  }
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

int
Player::get_coins() const
{
  return player_status->coins;
}

BonusType
Player::string_to_bonus(const std::string& bonus) const {
  BonusType type = NO_BONUS;

  if(bonus == "grow") {
    type = GROWUP_BONUS;
  } else if(bonus == "fireflower") {
    type = FIRE_BONUS;
  } else if(bonus == "iceflower") {
    type = ICE_BONUS;
  } else if(bonus == "airflower") {
    type = AIR_BONUS;
  } else if(bonus == "earthflower") {
    type = EARTH_BONUS;
  } else if(bonus == "none") {
    type = NO_BONUS;
  } else {
    std::ostringstream msg;
    msg << "Unknown bonus type "  << bonus;
    throw std::runtime_error(msg.str());
  }

  return type;
}

bool
Player::add_bonus(const std::string& bonustype)
{
  return add_bonus( string_to_bonus(bonustype) );
}

bool
Player::set_bonus(const std::string& bonustype)
{
  return set_bonus( string_to_bonus(bonustype) );
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
    if (player_status->bonus != NO_BONUS)
      return true;
  }

  return set_bonus(type, animate);
}

bool
Player::set_bonus(BonusType type, bool animate)
{
  if(dying) {
    return false;
  }

  if((player_status->bonus == NO_BONUS) && (type != NO_BONUS)) {
    if (!adjust_height(BIG_TUX_HEIGHT)) {
      log_debug << "Can't adjust Tux height" << std::endl;
      return false;
    }
    if(animate) {
      growing = true;
      sprite->set_action((dir == LEFT)?"grow-left":"grow-right", 1);
    }
    if (climbing) stop_climbing(*climbing);
  }

  if (type == NO_BONUS) {
    if (!adjust_height(SMALL_TUX_HEIGHT)) {
      log_debug << "Can't adjust Tux height" << std::endl;
      return false;
    }
    if (does_buttjump) does_buttjump = false;
  }

  if ((type == NO_BONUS) || (type == GROWUP_BONUS)) {
    Vector ppos = Vector((bbox.p1.x + bbox.p2.x) / 2, bbox.p1.y);
    Vector pspeed = Vector(((dir == LEFT) ? 100 : -100), -300);
    Vector paccel = Vector(0, 1000);
    std::string action = (dir == LEFT) ? "left" : "right";
    std::string particle_name = "";

    if ((player_status->bonus == FIRE_BONUS) && (animate)) {
      // visually lose helmet
      if (g_config->christmas_mode) {
        particle_name = "santatux-hat";
      }
      else {
        particle_name = "firetux-helmet";
      }
    }
    if ((player_status->bonus == ICE_BONUS) && (animate)) {
      // visually lose cap
      particle_name = "icetux-cap";
    }
    if ((player_status->bonus == AIR_BONUS) && (animate)) {
      // visually lose hat
      particle_name = "airtux-hat";
    }
    if ((player_status->bonus == EARTH_BONUS) && (animate)) {
      // visually lose hard-hat
      particle_name = "earthtux-hardhat";
    }
    if(!particle_name.empty() && animate) {
      Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/" + particle_name + ".sprite", action, ppos, ANCHOR_TOP, pspeed, paccel, LAYER_OBJECTS - 1));
    }
    if(climbing) stop_climbing(*climbing);

    player_status->max_fire_bullets = 0;
    player_status->max_ice_bullets = 0;
    player_status->max_air_time = 0;
    player_status->max_earth_time = 0;
  }
  if (type == FIRE_BONUS) player_status->max_fire_bullets++;
  if (type == ICE_BONUS) player_status->max_ice_bullets++;
  if (type == AIR_BONUS) player_status->max_air_time++;
  if (type == EARTH_BONUS) player_status->max_earth_time++;

  player_status->bonus = type;
  return true;
}

void
Player::set_visible(bool visible_)
{
  this->visible = visible_;
  if( visible_ )
    set_group(COLGROUP_MOVING);
  else
    set_group(COLGROUP_DISABLED);
}

bool
Player::get_visible() const
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
  if (Editor::is_active()) {
    return;
  }

  if(!visible)
    return;

  // if Tux is above camera, draw little "air arrow" to show where he is x-wise
  if (Sector::current() && Sector::current()->camera && (bbox.p2.y - 16 < Sector::current()->camera->get_translation().y)) {
    float px = bbox.p1.x + (bbox.p2.x - bbox.p1.x - airarrow.get()->get_width()) / 2;
    float py = Sector::current()->camera->get_translation().y;
    py += std::min(((py - (bbox.p2.y + 16)) / 4), 16.0f);
    context.draw_surface(airarrow, Vector(px, py), LAYER_HUD - 1);
  }

  std::string sa_prefix = "";
  std::string sa_postfix = "";

  if (player_status->bonus == GROWUP_BONUS)
    sa_prefix = "big";
  else if (player_status->bonus == FIRE_BONUS)
    if(g_config->christmas_mode)
      sa_prefix = "santa";
    else
      sa_prefix = "fire";
  else if (player_status->bonus == ICE_BONUS)
    sa_prefix = "ice";
  else if (player_status->bonus == AIR_BONUS)
    sa_prefix = "air";
  else if (player_status->bonus == EARTH_BONUS)
    sa_prefix = "earth";
  else
    sa_prefix = "small";

  if(dir == LEFT)
    sa_postfix = "-left";
  else
    sa_postfix = "-right";

  /* Set Tux sprite action */
  if(dying) {
    sprite->set_action("gameover");
  }
  else if (growing) {
    sprite->set_action_continued("grow"+sa_postfix);
    // while growing, do not change action
    // do_duck() will take care of cancelling growing manually
    // update() will take care of cancelling when growing completed
  }
  else if (stone) {
    sprite->set_action(sprite->get_action()+"-stone");
  }
  else if (climbing) {
    sprite->set_action(sa_prefix+"-climbing"+sa_postfix);
  }
  else if (backflipping) {
    sprite->set_action(sa_prefix+"-backflip"+sa_postfix);
  }
  else if (duck && is_big()) {
    sprite->set_action(sa_prefix+"-duck"+sa_postfix);
  }
  else if (skidding_timer.started() && !skidding_timer.check()) {
    sprite->set_action(sa_prefix+"-skid"+sa_postfix);
  }
  else if (kick_timer.started() && !kick_timer.check()) {
    sprite->set_action(sa_prefix+"-kick"+sa_postfix);
  }
  else if ((wants_buttjump || does_buttjump) && is_big()) {
    sprite->set_action(sa_prefix+"-buttjump"+sa_postfix);
  }
  else if (!on_ground() || fall_mode != ON_GROUND) {
    if(physic.get_velocity_x() != 0 || fall_mode != ON_GROUND) {
        sprite->set_action(sa_prefix+"-jump"+sa_postfix);
    }
  }
  else {
    if (fabsf(physic.get_velocity_x()) < 1.0f) {
      // Determine which idle stage we're at
      if (sprite->get_action().find("-stand-") == std::string::npos && sprite->get_action().find("-idle-") == std::string::npos) {
        idle_stage = 0;
        idle_timer.start(IDLE_TIME[idle_stage]/1000.0f);

        sprite->set_action_continued(sa_prefix+("-" + IDLE_STAGES[idle_stage])+sa_postfix);
      }
      else if (idle_timer.check() || (IDLE_TIME[idle_stage] == 0 && sprite->animation_done())) {
        idle_stage++;
        if (idle_stage >= IDLE_STAGE_COUNT)
          idle_stage = 1;

        idle_timer.start(IDLE_TIME[idle_stage]/1000.0f);

        if (IDLE_TIME[idle_stage] == 0)
          sprite->set_action(sa_prefix+("-" + IDLE_STAGES[idle_stage])+sa_postfix, 1);
        else
          sprite->set_action(sa_prefix+("-" + IDLE_STAGES[idle_stage])+sa_postfix);
      }
      else {
        sprite->set_action_continued(sa_prefix+("-" + IDLE_STAGES[idle_stage])+sa_postfix);
      }
    }
    else {
      if(fabsf(physic.get_velocity_x()) > MAX_WALK_XM && !is_big()) {
        sprite->set_action(sa_prefix+"-run"+sa_postfix);
      } else {
        sprite->set_action(sa_prefix+"-walk"+sa_postfix);
      }
    }
  }

  /* Set Tux powerup sprite action */
  if (player_status->bonus == EARTH_BONUS) {
    powersprite->set_action(sprite->get_action());
    lightsprite->set_action(sprite->get_action());
  } else if (player_status->bonus == AIR_BONUS) {
    powersprite->set_action(sprite->get_action());
  } else if (player_status->bonus == FIRE_BONUS && g_config->christmas_mode) {
    powersprite->set_action(sprite->get_action());
  }

  /*
  // Tux is holding something
  if ((grabbed_object != 0 && physic.get_velocity_y() == 0) ||
  (shooting_timer.get_timeleft() > 0 && !shooting_timer.check())) {
  if (duck) {
  } else {
  }
  }
  */

  /* Draw Tux */
  if (safe_timer.started() && size_t(game_time*40)%2)
    ;  // don't draw Tux
  else if (player_status->bonus == EARTH_BONUS){ // draw special effects with earthflower bonus
    // shake at end of maximum stone duration
    Vector shake_delta = (stone && ability_timer.get_timeleft() < 1.0f) ? Vector(graphicsRandom.rand(-3,3), 0) : Vector(0,0);
    sprite->draw(context, get_pos() + shake_delta, LAYER_OBJECTS + 1);
    // draw hardhat
    powersprite->draw(context, get_pos() + shake_delta, LAYER_OBJECTS + 1);
    // light
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, get_pos(), 0);
    context.pop_target();
    // give an indicator that stone form cannot be used for a while
    if (cooldown_timer.started() && graphicsRandom.rand(0, 4) == 0) {
      float px = graphicsRandom.randf(bbox.p1.x, bbox.p2.x);
      float py = bbox.p2.y+8;
      Vector ppos = Vector(px, py);
      Sector::current()->add_object(std::make_shared<SpriteParticle>(
        "images/objects/particles/sparkle.sprite", "dark",
        ppos, ANCHOR_MIDDLE, Vector(0, 0), Vector(0, 0), LAYER_OBJECTS+1+5));
    }
  }
  else {
    if(dying)
      sprite->draw(context, get_pos(), Sector::current()->get_foremost_layer());
    else
      sprite->draw(context, get_pos(), LAYER_OBJECTS + 1);

    if (player_status->bonus == AIR_BONUS)
      powersprite->draw(context, get_pos(), LAYER_OBJECTS + 1);
    else if(player_status->bonus == FIRE_BONUS && g_config->christmas_mode) {
      powersprite->draw(context, get_pos(), LAYER_OBJECTS + 1);
    }
  }

}

void
Player::collision_tile(uint32_t tile_attributes)
{
  if(tile_attributes & Tile::HURTS)
    kill(false);

#ifdef SWIMMING
  if( swimming ){
    if( tile_attributes & Tile::WATER ){
      no_water = false;
    } else {
      swimming = false;
    }
  } else {
    if( tile_attributes & Tile::WATER ){
      swimming = true;
      no_water = false;
      SoundManager::current()->play( "sounds/splash.wav" );
    }
  }
#endif

  if(tile_attributes & Tile::ICE) {
    ice_this_frame = true;
    on_ice = true;
  }
}

void
Player::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom) {
    if(physic.get_velocity_y() > 0)
      physic.set_velocity_y(0);

    on_ground_flag = true;
    floor_normal = hit.slope_normal;

    // Butt Jump landed
    if (does_buttjump) {
      does_buttjump = false;
      physic.set_velocity_y(-300);
      on_ground_flag = false;
      Sector::current()->add_object(std::make_shared<Particles>(
                                      bbox.p2,
                                      50, 70, 260, 280, Vector(0, 300), 3,
                                      Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1));
      Sector::current()->add_object(std::make_shared<Particles>(
                                      Vector(bbox.p1.x, bbox.p2.y),
                                      -70, -50, 260, 280, Vector(0, 300), 3,
                                      Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1));
      Sector::current()->camera->shake(.1f, 0, 5);
    }

  } else if(hit.top) {
    if(physic.get_velocity_y() < 0)
      physic.set_velocity_y(.2f);
  }

  if((hit.left || hit.right) && hit.slope_normal.x == 0) {
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

  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    return ABORT_MOVE;
  }

  if(hit.left || hit.right) {
    try_grab(); //grab objects right now, in update it will be too late
  }
  assert(dynamic_cast<MovingObject*> (&other) != NULL);
  MovingObject* moving_object = static_cast<MovingObject*> (&other);
  if(moving_object->get_group() == COLGROUP_TOUCHABLE) {
    TriggerBase* trigger = dynamic_cast<TriggerBase*> (&other);
    if(trigger && !deactivated) {
      if(controller->pressed(Controller::UP))
        trigger->event(*this, TriggerBase::EVENT_ACTIVATE);
    }

    return FORCE_MOVE;
  }

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy != NULL) {
    if(safe_timer.started() || invincible_timer.started())
      return FORCE_MOVE;
    if(stone)
      return ABORT_MOVE;

    return CONTINUE;
  }

  return CONTINUE;
}

void
Player::make_invincible()
{
  SoundManager::current()->play("sounds/invincible_start.ogg");
  invincible_timer.start(TUX_INVINCIBLE_TIME);
  Sector::current()->play_music(HERRING_MUSIC);
}

/* Kill Player! */
void
Player::kill(bool completely)
{
  if(dying || deactivated || is_winning() )
    return;

  if(!completely && (safe_timer.started() || invincible_timer.started() || stone))
    return;

  growing = false;

  if (climbing) stop_climbing(*climbing);

  physic.set_velocity_x(0);

  sprite->set_angle(0.0f);
  powersprite->set_angle(0.0f);
  lightsprite->set_angle(0.0f);

  if(!completely && is_big()) {
    SoundManager::current()->play("sounds/hurt.wav");

    if(player_status->bonus == FIRE_BONUS
      || player_status->bonus == ICE_BONUS
      || player_status->bonus == AIR_BONUS
      || player_status->bonus == EARTH_BONUS) {
      safe_timer.start(TUX_SAFE_TIME);
      set_bonus(GROWUP_BONUS, true);
    } else if(player_status->bonus == GROWUP_BONUS) {
      safe_timer.start(TUX_SAFE_TIME /* + GROWING_TIME */);
      duck = false;
      backflipping = false;
      sprite->set_angle(0.0f);
      powersprite->set_angle(0.0f);
      lightsprite->set_angle(0.0f);
      set_bonus(NO_BONUS, true);
    }
  } else {
    SoundManager::current()->play("sounds/kill.wav");

    // do not die when in edit mode
    if (edit_mode) {
      set_ghost_mode(true);
      return;
    }

    if (player_status->coins >= 25 && !GameSession::current()->get_reset_point_sectorname().empty())
    {
      for (int i = 0; i < 5; i++)
      {
        // the numbers: starting x, starting y, velocity y
        Sector::current()->add_object(std::make_shared<FallingCoin>(get_pos() +
                                                      Vector(graphicsRandom.rand(5), graphicsRandom.rand(-32,18)),
                                                      graphicsRandom.rand(-100,100)));
      }
      player_status->coins -= std::max(player_status->coins/10, 25);
    }
    else
    {
      GameSession::current()->set_reset_point("", Vector());
    }
    physic.enable_gravity(true);
    physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
    safe_timer.stop();
    invincible_timer.stop();
    physic.set_acceleration(0, 0);
    physic.set_velocity(0, -700);
    set_bonus(NO_BONUS, true);
    dying = true;
    dying_timer.start(3.0);
    set_group(COLGROUP_DISABLED);

    // TODO: need nice way to handle players dying in co-op mode
    Sector::current()->effect->fade_out(3.0);
    SoundManager::current()->pause_music(3.0);
  }
}

void
Player::move(const Vector& vector)
{
  set_pos(vector);

  // Reset size to get correct hitbox if Tux was eg. ducked before moving
  if(is_big())
    set_size(TUX_WIDTH, BIG_TUX_HEIGHT);
  else
    set_size(TUX_WIDTH, SMALL_TUX_HEIGHT);
  duck = false;
  backflipping = false;
  sprite->set_angle(0.0f);
  powersprite->set_angle(0.0f);
  lightsprite->set_angle(0.0f);
  last_ground_y = vector.y;
  if (climbing) stop_climbing(*climbing);

  physic.reset();
}

void
Player::check_bounds()
{
  /* Keep tux in sector bounds: */
  if (get_pos().x < 0) {
    // Lock Tux to the size of the level, so that he doesn't fall off
    // the left side
    set_pos(Vector(0, get_pos().y));
  }

  if (bbox.get_right() > Sector::current()->get_width()) {
    // Lock Tux to the size of the level, so that he doesn't fall off
    // the right side
    set_pos(Vector(Sector::current()->get_width() - bbox.get_width(), bbox.p1.y));
  }

  /* fallen out of the level? */
  if ((get_pos().y > Sector::current()->get_height()) && (!ghost_mode)) {
    kill(true);
    return;
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
  if (end_speed.x > 0)
    physic.set_velocity_x(std::min(physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.x < 0)
    physic.set_velocity_x(std::max(physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.y > 0)
    physic.set_velocity_y(std::min(physic.get_velocity_y() + velocity.y, end_speed.y));
  if (end_speed.y < 0)
    physic.set_velocity_y(std::max(physic.get_velocity_y() + velocity.y, end_speed.y));
}

Vector
Player::get_velocity() const
{
  return physic.get_velocity();
}

void
Player::bounce(BadGuy& )
{
  if(!(player_status->bonus == AIR_BONUS))
    physic.set_velocity_y(controller->hold(Controller::JUMP) ? -520 : -300);
  else {
    physic.set_velocity_y(controller->hold(Controller::JUMP) ? -580 : -340);
    ability_time = player_status->max_air_time * GLIDE_TIME_PER_FLOWER;
  }
}

//scripting Functions Below

void
Player::deactivate()
{
  if (deactivated)
    return;
  deactivated = true;
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  physic.set_acceleration_x(0);
  physic.set_acceleration_y(0);
  if (climbing) stop_climbing(*climbing);
}

void
Player::activate()
{
  if (!deactivated)
    return;
  deactivated = false;
}

void Player::walk(float speed)
{
  physic.set_velocity_x(speed);
}

void Player::set_dir(bool right)
{
  dir = right ? RIGHT : LEFT;
}

void
Player::set_ghost_mode(bool enable)
{
  if (ghost_mode == enable)
    return;

  if (climbing) stop_climbing(*climbing);

  if (grabbed_object) {
    grabbed_object->ungrab(*this, dir);
    grabbed_object = NULL;
  }

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

void
Player::set_edit_mode(bool enable)
{
  edit_mode = enable;
}

void
Player::start_climbing(Climbable& climbable)
{
  if (climbing) return;

  climbing = &climbable;
  physic.enable_gravity(false);
  physic.set_velocity(0, 0);
  physic.set_acceleration(0, 0);
  if (backflipping) {
    backflipping = false;
    backflip_direction = 0;
    sprite->set_angle(0.0f);
    powersprite->set_angle(0.0f);
    lightsprite->set_angle(0.0f);
    do_standup();
  }
}

void
Player::stop_climbing(Climbable& /*climbable*/)
{
  if (!climbing) return;

  climbing = 0;

  if (grabbed_object) {
    grabbed_object->ungrab(*this, dir);
    grabbed_object = NULL;
  }

  physic.enable_gravity(true);
  physic.set_velocity(0, 0);
  physic.set_acceleration(0, 0);

  if ((controller->hold(Controller::JUMP)) || (controller->hold(Controller::UP))) {
    on_ground_flag = true;
    // TODO: This won't help. Why?
    do_jump(-300);
  }
}

void
Player::handle_input_climbing()
{
  if (!climbing) {
    log_warning << "handle_input_climbing called with climbing set to 0. Input handling skipped" << std::endl;
    return;
  }

  float vx = 0;
  float vy = 0;
  if (controller->hold(Controller::LEFT)) {
    dir = LEFT;
    vx -= MAX_CLIMB_XM;
  }
  if (controller->hold(Controller::RIGHT)) {
    dir = RIGHT;
    vx += MAX_CLIMB_XM;
  }
  if (controller->hold(Controller::UP)) {
    vy -= MAX_CLIMB_YM;
  }
  if (controller->hold(Controller::DOWN)) {
    vy += MAX_CLIMB_YM;
  }
  if (controller->hold(Controller::JUMP)) {
    if (can_jump) {
      stop_climbing(*climbing);
      return;
    }
  } else {
    can_jump = true;
  }
  if (controller->hold(Controller::ACTION)) {
    stop_climbing(*climbing);
    return;
  }
  physic.set_velocity(vx, vy);
  physic.set_acceleration(0, 0);
}

/* EOF */
