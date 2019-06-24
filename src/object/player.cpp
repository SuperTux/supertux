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
#include "control/codecontroller.hpp"
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
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "trigger/trigger_base.hpp"
#include "video/surface.hpp"

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

Player::Player(PlayerId id, PlayerStatus* _player_status, const std::string& name_):
	m_id(id),
	ExposedObject<Player, scripting::Player>(this),
	m_deactivated(false),
	m_controller(InputManager::current()->get_controller(id)),
	m_scripting_controller(new CodeController()),
	m_player_status(_player_status),
	m_duck(false),
	m_dead(false),
	m_dying(false),
	m_winning(false),
	m_backflipping(false),
	m_backflip_direction(0),
	m_peekingX(AUTO),
	m_peekingY(AUTO),
	m_ability_time(),
	m_stone(false),
	m_swimming(false),
	m_speedlimit(0), //no special limit
	m_scripting_controller_old(0),
	m_jump_early_apex(false),
	m_on_ice(false),
	m_ice_this_frame(false),
	m_lightsprite(SpriteManager::current()->create("images/creatures/tux/light.sprite")),
	m_powersprite(SpriteManager::current()->create("images/creatures/tux/powerups.sprite")),
	m_dir(RIGHT),
	m_old_dir(m_dir),
	m_last_ground_y(0),
	m_fall_mode(ON_GROUND),
	m_on_ground_flag(false),
	m_jumping(false),
	m_can_jump(true),
	m_jump_button_timer(),
	m_wants_buttjump(false),
	m_does_buttjump(false),
	m_invincible_timer(),
	m_skidding_timer(),
	m_safe_timer(),
	m_kick_timer(),
	m_shooting_timer(),
	m_ability_timer(),
	m_cooldown_timer(),
	m_dying_timer(),
	m_second_growup_sound_timer(),
	m_growing(false),
	m_backflip_timer(),
	m_physic(),
	m_visible(true),
	m_grabbed_object(NULL),
	// if/when we have complete penny gfx, we can
	// load those instead of Tux's sprite in the
	// constructor
	m_sprite(SpriteManager::current()->create("images/creatures/tux/tux.sprite")),
	m_airarrow(Surface::from_file("images/engine/hud/airarrow.png")),
	m_floor_normal(),
	m_ghost_mode(false),
	m_edit_mode(false),
	m_unduck_hurt_timer(),
	m_idle_timer(),
	m_idle_stage(0),
	m_climbing(0),
	camera(std::make_shared<Camera>(Sector::current(), this, "Camera"))
{
	m_name = name_;
	m_idle_timer.start(static_cast<float>(IDLE_TIME[0]) / 1000.0f);

	SoundManager::current()->preload("sounds/bigjump.wav");
	SoundManager::current()->preload("sounds/jump.wav");
	SoundManager::current()->preload("sounds/hurt.wav");
	SoundManager::current()->preload("sounds/kill.wav");
	SoundManager::current()->preload("sounds/skid.wav");
	SoundManager::current()->preload("sounds/flip.wav");
	SoundManager::current()->preload("sounds/invincible_start.ogg");
	SoundManager::current()->preload("sounds/splash.wav");
	SoundManager::current()->preload("sounds/grow.wav");
	set_size(TUX_WIDTH, is_big() ? BIG_TUX_HEIGHT : SMALL_TUX_HEIGHT);

	m_sprite->set_angle(0.0f);
	m_powersprite->set_angle(0.0f);
	m_lightsprite->set_angle(0.0f);
	m_lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));

	m_physic.reset();
}


Player::~Player()
{
  if (m_climbing) stop_climbing(*m_climbing);
}

float
Player::get_speedlimit() const
{
  return m_speedlimit;
}

void
Player::set_speedlimit(float newlimit)
{
  m_speedlimit=newlimit;
}

void
Player::set_controller(Controller* controller_)
{
  m_controller = controller_;
}

void
Player::set_winning()
{
  if( ! is_winning() ){
    m_winning = true;
    m_invincible_timer.start(10000.0f);
  }
}

void
Player::use_scripting_controller(bool use_or_release)
{
  if ((use_or_release == true) && (m_controller != m_scripting_controller.get())) {
    m_scripting_controller_old = get_controller();
    set_controller(m_scripting_controller.get());
  }
  if ((use_or_release == false) && (m_controller == m_scripting_controller.get())) {
    set_controller(m_scripting_controller_old);
    m_scripting_controller_old = 0;
  }
}

void
Player::do_scripting_controller(const std::string& control, bool pressed)
{
  for(int i = 0; Controller::controlNames[i] != 0; ++i) {
    if(control == std::string(Controller::controlNames[i])) {
      m_scripting_controller->press(Controller::Control(i), pressed);
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
Player::trigger_sequence(const std::string& sequence_name, const SequenceData* data)
{
  trigger_sequence(string_to_sequence(sequence_name), data);
}

void
Player::trigger_sequence(Sequence seq, const SequenceData* data)
{
  if (m_climbing) stop_climbing(*m_climbing);
  stop_backflipping();
  GameSession::current()->start_sequence(seq, data);
}

void
Player::update(float elapsed_time)
{
  // Update camera position
  camera->update(elapsed_time);

  if( no_water ){
    m_swimming = false;
  }
  no_water = true;

  if(m_dying && m_dying_timer.check()) {
    Sector::current()->stop_looping_sounds();
    set_bonus(NO_BONUS, true);
    m_dead = true;
    return;
  }

  if(!m_dying && !m_deactivated)
    handle_input();

  /*
  // handle_input() calls apply_friction() when Tux is not walking, so we'll have to do this ourselves
  if (deactivated)
  apply_friction();
  */

  // extend/shrink tux collision rectangle so that we fall through/walk over 1
  // tile holes
  if(fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) {
    set_width(RUNNING_TUX_WIDTH);
  } else {
    set_width(TUX_WIDTH);
  }

  // on downward slopes, adjust vertical velocity so tux walks smoothly down
  if (on_ground() && !m_dying) {
    if(m_floor_normal.y != 0) {
      if ((m_floor_normal.x * m_physic.get_velocity_x()) >= 0) {
        m_physic.set_velocity_y(250);
      }
    }
  }

  // handle backflipping
  if (m_backflipping && !m_dying) {
    //prevent player from changing direction when backflipping
    m_dir = (m_backflip_direction == 1) ? LEFT : RIGHT;
    if (m_backflip_timer.started()) m_physic.set_velocity_x(100.0f * static_cast<float>(m_backflip_direction));
    //rotate sprite during flip
    m_sprite->set_angle(m_sprite->get_angle() + (m_dir==LEFT?1:-1) * elapsed_time * (360.0f / 0.5f));
    if (m_player_status->bonus == EARTH_BONUS || m_player_status->bonus == AIR_BONUS ||
        (m_player_status->bonus == FIRE_BONUS && g_config->christmas_mode)) {
      m_powersprite->set_angle(m_sprite->get_angle());
      if (m_player_status->bonus == EARTH_BONUS)
        m_lightsprite->set_angle(m_sprite->get_angle());
    }
  }

  // set fall mode...
  if(on_ground()) {
    m_fall_mode = ON_GROUND;
    m_last_ground_y = get_pos().y;
  } else {
    if(get_pos().y > m_last_ground_y)
      m_fall_mode = FALLING;
    else if(m_fall_mode == ON_GROUND)
      m_fall_mode = JUMPING;
  }

  // check if we landed
  if(on_ground()) {
    m_jumping = false;
    if (m_backflipping && (m_backflip_timer.get_timegone() > 0.15f)) {
      m_backflipping = false;
      m_backflip_direction = 0;
      m_physic.set_velocity_x(0);
      if (!m_stone) {
        m_sprite->set_angle(0.0f);
        m_powersprite->set_angle(0.0f);
        m_lightsprite->set_angle(0.0f);
      }

      // if controls are currently deactivated, we take care of standing up ourselves
      if (m_deactivated)
        do_standup();
    }
    if (m_player_status->bonus == AIR_BONUS)
      m_ability_time = static_cast<float>(m_player_status->max_air_time) * GLIDE_TIME_PER_FLOWER;

    if(m_second_growup_sound_timer.check())
    {
      SoundManager::current()->play("sounds/grow.wav");
      m_second_growup_sound_timer.stop();
    }
  }

  // calculate movement for this frame
  movement = m_physic.get_movement(elapsed_time);

  if(m_grabbed_object != NULL && !m_dying) {
    position_grabbed_object();
  }

  if(m_grabbed_object != NULL && m_dying){
    m_grabbed_object->ungrab(*this, m_dir);
    m_grabbed_object = NULL;
  }

  if(!m_ice_this_frame && on_ground())
    m_on_ice = false;

  m_on_ground_flag = false;
  m_ice_this_frame = false;

  // when invincible, spawn particles
  if (m_invincible_timer.started())
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
                                      (m_invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING) ?
                                      // make every other a longer sparkle to make trail a bit fuzzy
                                      (size_t(game_time*20)%2) ? "small" : "medium"
                                      :
                                      "dark", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
    }
  }

  if (m_growing) {
    if (m_sprite->animation_done()) m_growing = false;
  }

  // when climbing animate only while moving
  if(m_climbing){
    if((m_physic.get_velocity_x()==0)&&(m_physic.get_velocity_y()==0))
      m_sprite->stop_animation();
    else
      m_sprite->set_animation_loops(-1);
  }

}

bool
Player::slightly_above_ground() const
{
  float abs_vy = std::abs(m_physic.get_velocity_y());
  float ground_y_delta = std::abs(m_last_ground_y - get_pos().y);
  return (abs_vy == 15.625 || abs_vy == 31.25) && ground_y_delta < 0.85;
}

bool
Player::on_ground() const
{
  return m_on_ground_flag || slightly_above_ground();
}

bool
Player::is_big() const
{
  if(m_player_status->bonus == NO_BONUS)
    return false;

  return true;
}

void
Player::apply_friction()
{
  if ((on_ground()) && (fabsf(m_physic.get_velocity_x()) < WALK_SPEED)) {
    m_physic.set_velocity_x(0);
    m_physic.set_acceleration_x(0);
  } else {
    float friction = WALK_ACCELERATION_X * (m_on_ice ? ICE_FRICTION_MULTIPLIER : NORMAL_FRICTION_MULTIPLIER);
    if(m_physic.get_velocity_x() < 0) {
      m_physic.set_acceleration_x(friction);
    } else if(m_physic.get_velocity_x() > 0) {
      m_physic.set_acceleration_x(-friction);
    } // no friction for physic.get_velocity_x() == 0
  }
}

void
Player::handle_horizontal_input()
{
  float vx = m_physic.get_velocity_x();
  float vy = m_physic.get_velocity_y();
  float ax = m_physic.get_acceleration_x();
  float ay = m_physic.get_acceleration_y();

  float dirsign = 0;
  if(!m_duck || m_physic.get_velocity_y() != 0) {
    if(m_controller->hold(Controller::LEFT) && !m_controller->hold(Controller::RIGHT)) {
      m_old_dir = m_dir;
      m_dir = LEFT;
      dirsign = -1;
    } else if(!m_controller->hold(Controller::LEFT)
              && m_controller->hold(Controller::RIGHT)) {
      m_old_dir = m_dir;
      m_dir = RIGHT;
      dirsign = 1;
    }
  }

  // do not run if we're holding something which slows us down
  if ( m_grabbed_object && m_grabbed_object->is_hampering() ) {
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
    if(vx >= MAX_RUN_XM + BONUS_RUN_XM *((m_player_status->bonus == AIR_BONUS) ? 1 : 0) && dirsign > 0) {
      vx = MAX_RUN_XM + BONUS_RUN_XM *((m_player_status->bonus == AIR_BONUS) ? 1 : 0);
      ax = 0;
    } else if(vx <= -MAX_RUN_XM - BONUS_RUN_XM *((m_player_status->bonus == AIR_BONUS) ? 1 : 0) && dirsign < 0) {
      vx = -MAX_RUN_XM - BONUS_RUN_XM *((m_player_status->bonus == AIR_BONUS) ? 1 : 0);
      ax = 0;
    }
  }

  // we can reach WALK_SPEED without any acceleration
  if(dirsign != 0 && fabsf(vx) < WALK_SPEED) {
    vx = dirsign * WALK_SPEED;
  }

  //Check speedlimit.
  if( m_speedlimit > 0 &&  vx * dirsign >= m_speedlimit ) {
    vx = dirsign * m_speedlimit;
    ax = 0;
  }

  // changing directions?
  if ((vx < 0 && dirsign >0) || (vx>0 && dirsign<0)) {
    if(on_ground()) {
      // let's skid!
      if(fabsf(vx)>SKID_XM && !m_skidding_timer.started()) {
        m_skidding_timer.start(SKID_TIME);
        SoundManager::current()->play("sounds/skid.wav");
        // dust some particles
        Sector::current()->add_object(
          std::make_shared<Particles>(
            Vector(m_dir == LEFT ? bbox.p2.x : bbox.p1.x, bbox.p2.y),
            m_dir == LEFT ? 50 : -70, m_dir == LEFT ? 70 : -50, 260, 280,
            Vector(0, 300), 3, Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1));

        ax *= 2.5f;
      } else {
        ax *= 2;
      }
    }
    else {
      // give Tux tighter air control
      ax *= 2.f;
    }
  }

  if(m_on_ice) {
    ax *= ICE_ACCELERATION_MULTIPLIER;
  }

  m_physic.set_velocity(vx, vy);
  m_physic.set_acceleration(ax, ay);

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
  if (m_duck)
    return;
  if (!is_big())
    return;

  if (m_physic.get_velocity_y() != 0)
    return;
  if (!on_ground())
    return;
  if (m_does_buttjump)
    return;

  if (adjust_height(DUCKED_TUX_HEIGHT)) {
    m_duck = true;
    m_growing = false;
    m_unduck_hurt_timer.stop();
  } else {
    // FIXME: what now?
  }
}

void
Player::do_standup() {
  if (!m_duck)
    return;
  if (!is_big())
    return;
  if (m_backflipping)
    return;
  if (m_stone)
    return;

  if (adjust_height(BIG_TUX_HEIGHT)) {
    m_duck = false;
    m_unduck_hurt_timer.stop();
  } else {
    // if timer is not already running, start it.
    if (m_unduck_hurt_timer.get_period() == 0) {
      m_unduck_hurt_timer.start(UNDUCK_HURT_TIME);
    }
    else if (m_unduck_hurt_timer.check()) {
      kill(false);
    }
  }

}

void
Player::do_backflip() {
  if (!m_duck)
    return;
  if (!on_ground())
    return;

  m_backflip_direction = (m_dir == LEFT)?(+1):(-1);
  m_backflipping = true;
  do_jump((m_player_status->bonus == AIR_BONUS) ? -720 : -580);
  SoundManager::current()->play("sounds/flip.wav");
  m_backflip_timer.start(TUX_BACKFLIP_TIME);
}

void
Player::do_jump(float yspeed) {
  if (!on_ground())
    return;

  m_physic.set_velocity_y(yspeed);
  //bbox.move(Vector(0, -1));
  m_jumping = true;
  m_on_ground_flag = false;
  m_can_jump = false;

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
  if (!m_jump_early_apex)
  {
    m_jump_early_apex = true;
    m_physic.set_gravity_modifier(JUMP_EARLY_APEX_FACTOR);
  }
}

void
Player::do_jump_apex()
{
  if (m_jump_early_apex)
  {
    m_jump_early_apex = false;
    m_physic.set_gravity_modifier(1.0f);
  }
}

void
Player::handle_vertical_input()
{
  // Press jump key
  if(m_controller->pressed(Controller::JUMP)) m_jump_button_timer.start(JUMP_GRACE_TIME);
  if(m_controller->hold(Controller::JUMP) && m_jump_button_timer.started() && m_can_jump) {
    m_jump_button_timer.stop();
    if (m_duck) {
      // when running, only jump a little bit; else do a backflip
      if ((m_physic.get_velocity_x() != 0) ||
          (m_controller->hold(Controller::LEFT)) ||
          (m_controller->hold(Controller::RIGHT)))
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
      if(m_player_status->bonus == AIR_BONUS)
        do_jump((fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) ? -620 : -580);
      else
        do_jump((fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) ? -580 : -520);
    }
    // airflower glide only when holding jump key
  } else  if (m_controller->hold(Controller::JUMP) && m_player_status->bonus == AIR_BONUS && m_physic.get_velocity_y() > MAX_GLIDE_YM) {
      if (m_ability_time > 0 && !m_ability_timer.started())
        m_ability_timer.start(m_ability_time);
      else if (m_ability_timer.started()) {
        // glide stops after some duration or if buttjump is initiated
        if ((m_ability_timer.get_timeleft() <= 0.05f) || m_controller->hold(Controller::DOWN)) {
          m_ability_time = 0;
          m_ability_timer.stop();
        } else {
          m_physic.set_velocity_y(MAX_GLIDE_YM);
          m_physic.set_acceleration_y(0);
        }
      }
    }


  // Let go of jump key
  else if(!m_controller->hold(Controller::JUMP)) {
    if (!m_backflipping && m_jumping && m_physic.get_velocity_y() < 0) {
      m_jumping = false;
      early_jump_apex();
    }
    if (m_player_status->bonus == AIR_BONUS && m_ability_timer.started()){
      m_ability_time = m_ability_timer.get_timeleft();
      m_ability_timer.stop();
    }
  }

  if(m_jump_early_apex && m_physic.get_velocity_y() >= 0) {
    do_jump_apex();
  }

  /* In case the player has pressed Down while in a certain range of air,
     enable butt jump action */
  if (m_controller->hold(Controller::DOWN) && !m_duck && is_big() && !on_ground()) {
    m_wants_buttjump = true;
    if (m_physic.get_velocity_y() >= BUTTJUMP_MIN_VELOCITY_Y) m_does_buttjump = true;
  }

  /* When Down is not held anymore, disable butt jump */
  if(!m_controller->hold(Controller::DOWN)) {
    m_wants_buttjump = false;
    m_does_buttjump = false;
  }

  // swimming
  m_physic.set_acceleration_y(0);
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
  if (m_ghost_mode) {
    handle_input_ghost();
    return;
  }
  if (m_climbing) {
    handle_input_climbing();
    return;
  }

  /* Peeking */
  if( m_controller->released( Controller::PEEK_LEFT ) || m_controller->released( Controller::PEEK_RIGHT ) ) {
    m_peekingX = AUTO;
  }
  if( m_controller->released( Controller::PEEK_UP ) || m_controller->released( Controller::PEEK_DOWN ) ) {
    m_peekingY = AUTO;
  }
  if( m_controller->pressed( Controller::PEEK_LEFT ) ) {
    m_peekingX = LEFT;
  }
  if( m_controller->pressed( Controller::PEEK_RIGHT ) ) {
    m_peekingX = RIGHT;
  }
  if(!m_backflipping && !m_jumping && on_ground()) {
    if( m_controller->pressed( Controller::PEEK_UP ) ) {
      m_peekingY = UP;
    } else if( m_controller->pressed( Controller::PEEK_DOWN ) ) {
      m_peekingY = DOWN;
    }
  }

  /* Handle horizontal movement: */
  if (!m_backflipping && !m_stone) handle_horizontal_input();

  /* Jump/jumping? */
  if (on_ground())
    m_can_jump = true;

  /* Handle vertical movement: */
  if (!m_stone) handle_vertical_input();

  /* Shoot! */
  auto sector = Sector::current();
  auto active_bullets = sector->get_active_bullets();
  if (m_controller->pressed(Controller::ACTION) && (m_player_status->bonus == FIRE_BONUS || m_player_status->bonus == ICE_BONUS)) {
    if((m_player_status->bonus == FIRE_BONUS &&
      active_bullets < m_player_status->max_fire_bullets) ||
      (m_player_status->bonus == ICE_BONUS &&
      active_bullets < m_player_status->max_ice_bullets))
    {
      Vector pos = get_pos() + ((m_dir == LEFT)? Vector(0, bbox.get_height()/2) : Vector(32, bbox.get_height()/2));
      auto new_bullet = std::make_shared<Bullet>(pos, m_physic.get_velocity_x(), m_dir, m_player_status->bonus);
      sector->add_object(new_bullet);

      SoundManager::current()->play("sounds/shoot.wav");
      m_shooting_timer.start(SHOOTING_TIME);
    }
  }

  /* Turn to Stone */
  if (m_controller->pressed(Controller::DOWN) && m_player_status->bonus == EARTH_BONUS && !m_cooldown_timer.started() && on_ground()) {
    if (m_controller->hold(Controller::ACTION) && !m_ability_timer.started()) {
      m_ability_timer.start(static_cast<float>(m_player_status->max_earth_time) * STONE_TIME_PER_FLOWER);
      m_powersprite->stop_animation();
      m_stone = true;
      m_physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
    }
  }

  if (m_stone)
    apply_friction();

  /* Revert from Stone */
  if (m_stone && (!m_controller->hold(Controller::ACTION) || m_ability_timer.get_timeleft() <= 0.5f)) {
    m_cooldown_timer.start(m_ability_timer.get_timegone()/2.0f); //The longer stone form is used, the longer until it can be used again
    m_ability_timer.stop();
    m_sprite->set_angle(0.0f);
    m_powersprite->set_angle(0.0f);
    m_lightsprite->set_angle(0.0f);
    m_stone = false;
    for (int i = 0; i < 8; i++)
    {
      Vector ppos = Vector(bbox.get_left() + 8.0f + 16.0f * static_cast<float>(i / 4),
                           bbox.get_top() + 16.0f * static_cast<float>(i % 4));
      float grey = graphicsRandom.randf(.4f, .8f);
      Color pcolor = Color(grey, grey, grey);
      Sector::current()->add_object(std::make_shared<Particles>(ppos, -60, 240, 42, 81, Vector(0.0f, 500.0f),
                                                                8, pcolor, 4 + graphicsRandom.randf(-0.4f, 0.4f),
                                                                0.8f + graphicsRandom.randf(0.0f, 0.4f), LAYER_OBJECTS + 2));
    }
  }

  /* Duck or Standup! */
  if (m_controller->hold(Controller::DOWN) && !m_stone) {
    do_duck();
  } else {
    do_standup();
  }

  /* grabbing */
  try_grab();

  if(!m_controller->hold(Controller::ACTION) && m_grabbed_object) {
    auto moving_object = dynamic_cast<MovingObject*> (m_grabbed_object);
    if(moving_object) {
      // move the grabbed object a bit away from tux
      Rectf grabbed_bbox = moving_object->get_bbox();
      Rectf dest_;
      dest_.p2.y = bbox.get_top() + bbox.get_height()*0.66666f;
      dest_.p1.y = dest_.p2.y - grabbed_bbox.get_height();
      if(m_dir == LEFT) {
        dest_.p2.x = bbox.get_left() - 1;
        dest_.p1.x = dest_.p2.x - grabbed_bbox.get_width();
      } else {
        dest_.p1.x = bbox.get_right() + 1;
        dest_.p2.x = dest_.p1.x + grabbed_bbox.get_width();
      }
      if(sector->is_free_of_tiles(dest_, true) &&
         sector->is_free_of_statics(dest_, moving_object, true)) {
        moving_object->set_pos(dest_.p1);
        if(m_controller->hold(Controller::UP)) {
          m_grabbed_object->ungrab(*this, UP);
        } else {
          m_grabbed_object->ungrab(*this, m_dir);
        }
        m_grabbed_object = NULL;
      }
    } else {
      log_debug << "Non MovingObject grabbed?!?" << std::endl;
    }
  }

  /* stop backflipping at will */
  if( m_backflipping && ( !m_controller->hold(Controller::JUMP) && !m_backflip_timer.started()) ){
    stop_backflipping();
  }
}

void
Player::position_grabbed_object()
{
  auto moving_object = dynamic_cast<MovingObject*>(m_grabbed_object);
  assert(moving_object);
  auto object_bbox = moving_object->get_bbox();

  // Position where we will hold the lower-inner corner
  Vector pos(bbox.get_left() + bbox.get_width()/2,
      bbox.get_top() + bbox.get_height()*0.66666f);

  // Adjust to find the grabbed object's upper-left corner
  if (m_dir == LEFT)
    pos.x -= object_bbox.get_width();
  pos.y -= object_bbox.get_height();

  m_grabbed_object->grab(*this, pos, m_dir);
}

void
Player::try_grab()
{
  if(m_controller->hold(Controller::ACTION) && !m_grabbed_object
     && !m_duck) {
    auto sector = Sector::current();
    Vector pos;
    if(m_dir == LEFT) {
      pos = Vector(bbox.get_left() - 5, bbox.get_bottom() - 16);
    } else {
      pos = Vector(bbox.get_right() + 5, bbox.get_bottom() - 16);
    }

    for(auto& portable : sector->m_portables) {
      if(!portable->is_portable())
        continue;

      // make sure the Portable is a MovingObject
      auto moving_object = dynamic_cast<MovingObject*>(portable);
      assert(moving_object);

      // make sure the Portable isn't currently non-solid
      if(moving_object->get_group() == COLGROUP_DISABLED) continue;

      // check if we are within reach
      if(moving_object->get_bbox().contains(pos)) {
        if (m_climbing) stop_climbing(*m_climbing);
        m_grabbed_object = portable;
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
  if (m_controller->hold(Controller::LEFT)) {
    m_dir = LEFT;
    vx -= MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Controller::RIGHT)) {
    m_dir = RIGHT;
    vx += MAX_RUN_XM * 2;
  }
  if ((m_controller->hold(Controller::UP)) || (m_controller->hold(Controller::JUMP))) {
    vy -= MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Controller::DOWN)) {
    vy += MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Controller::ACTION)) {
    set_ghost_mode(false);
  }
  m_physic.set_velocity(vx, vy);
  m_physic.set_acceleration(0, 0);
}

void
Player::add_coins(int count)
{
  m_player_status->add_coins(count);
}

int
Player::get_coins() const
{
  return m_player_status->coins;
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
    if (m_player_status->bonus != NO_BONUS)
      return true;
  }

  return set_bonus(type, animate);
}

bool
Player::set_bonus(BonusType type, bool animate)
{
  if(m_dying) {
    return false;
  }

  if((m_player_status->bonus == NO_BONUS) && (type != NO_BONUS)) {
    if (!adjust_height(BIG_TUX_HEIGHT)) {
      log_debug << "Can't adjust Tux height" << std::endl;
      return false;
    }
    if(animate) {
      m_growing = true;
      m_sprite->set_action((m_dir == LEFT)?"grow-left":"grow-right", 1);
    }
    if (m_climbing) stop_climbing(*m_climbing);
  }

  if (type == NO_BONUS) {
    if (!adjust_height(SMALL_TUX_HEIGHT)) {
      log_debug << "Can't adjust Tux height" << std::endl;
      return false;
    }
    if (m_does_buttjump) m_does_buttjump = false;
  }

  if ((type == NO_BONUS) || (type == GROWUP_BONUS)) {
    Vector ppos = Vector((bbox.p1.x + bbox.p2.x) / 2, bbox.p1.y);
    Vector pspeed = Vector(((m_dir == LEFT) ? 100 : -100), -300);
    Vector paccel = Vector(0, 1000);
    std::string action = (m_dir == LEFT) ? "left" : "right";
    std::string particle_name = "";

    if ((m_player_status->bonus == FIRE_BONUS) && (animate)) {
      // visually lose helmet
      if (g_config->christmas_mode) {
        particle_name = "santatux-hat";
      }
      else {
        particle_name = "firetux-helmet";
      }
    }
    if ((m_player_status->bonus == ICE_BONUS) && (animate)) {
      // visually lose cap
      particle_name = "icetux-cap";
    }
    if ((m_player_status->bonus == AIR_BONUS) && (animate)) {
      // visually lose hat
      particle_name = "airtux-hat";
    }
    if ((m_player_status->bonus == EARTH_BONUS) && (animate)) {
      // visually lose hard-hat
      particle_name = "earthtux-hardhat";
    }
    if(!particle_name.empty() && animate) {
      Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/" + particle_name + ".sprite", action, ppos, ANCHOR_TOP, pspeed, paccel, LAYER_OBJECTS - 1));
    }
    if(m_climbing) stop_climbing(*m_climbing);

    m_player_status->max_fire_bullets = 0;
    m_player_status->max_ice_bullets = 0;
    m_player_status->max_air_time = 0;
    m_player_status->max_earth_time = 0;
  }
  if (type == FIRE_BONUS) m_player_status->max_fire_bullets++;
  if (type == ICE_BONUS) m_player_status->max_ice_bullets++;
  if (type == AIR_BONUS) m_player_status->max_air_time++;
  if (type == EARTH_BONUS) m_player_status->max_earth_time++;

  if(!m_second_growup_sound_timer.started() &&
     type > GROWUP_BONUS && type != m_player_status->bonus)
  {
    m_second_growup_sound_timer.start(0.5);
  }

  m_player_status->bonus = type;
  return true;
}

void
Player::set_visible(bool visible_)
{
  m_visible = visible_;
  if( visible_ )
    set_group(COLGROUP_MOVING);
  else
    set_group(COLGROUP_DISABLED);
}

bool
Player::get_visible() const
{
  return m_visible;
}

void
Player::kick()
{
  m_kick_timer.start(KICK_TIME);
}

void
Player::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    return;
  }

  if(!m_visible)
    return;

  camera->draw(context);

  // if Tux is above camera, draw little "air arrow" to show where he is x-wise
  if (Sector::current() && camera && (bbox.p2.y - 16 < camera->get_translation().y)) {
    float px = bbox.p1.x + (bbox.p2.x - bbox.p1.x - static_cast<float>(m_airarrow.get()->get_width())) / 2.0f;
    float py = camera->get_translation().y;
    py += std::min(((py - (bbox.p2.y + 16)) / 4), 16.0f);
    context.color().draw_surface(m_airarrow, Vector(px, py), LAYER_HUD - 1);
  }

  std::string sa_prefix = "";
  std::string sa_postfix = "";

  if (m_player_status->bonus == GROWUP_BONUS)
    sa_prefix = "big";
  else if (m_player_status->bonus == FIRE_BONUS)
    if(g_config->christmas_mode)
      sa_prefix = "santa";
    else
      sa_prefix = "fire";
  else if (m_player_status->bonus == ICE_BONUS)
    sa_prefix = "ice";
  else if (m_player_status->bonus == AIR_BONUS)
    sa_prefix = "air";
  else if (m_player_status->bonus == EARTH_BONUS)
    sa_prefix = "earth";
  else
    sa_prefix = "small";

  if(m_dir == LEFT)
    sa_postfix = "-left";
  else
    sa_postfix = "-right";

  /* Set Tux sprite action */
  if(m_dying) {
    m_sprite->set_action("gameover");
  }
  else if (m_growing) {
    m_sprite->set_action_continued("grow"+sa_postfix);
    // while growing, do not change action
    // do_duck() will take care of cancelling growing manually
    // update() will take care of cancelling when growing completed
  }
  else if (m_stone) {
    m_sprite->set_action(m_sprite->get_action()+"-stone");
  }
  else if (m_climbing) {
    m_sprite->set_action(sa_prefix+"-climbing"+sa_postfix);
  }
  else if (m_backflipping) {
    m_sprite->set_action(sa_prefix+"-backflip"+sa_postfix);
  }
  else if (m_duck && is_big()) {
    m_sprite->set_action(sa_prefix+"-duck"+sa_postfix);
  }
  else if (m_skidding_timer.started() && !m_skidding_timer.check()) {
    m_sprite->set_action(sa_prefix+"-skid"+sa_postfix);
  }
  else if (m_kick_timer.started() && !m_kick_timer.check()) {
    m_sprite->set_action(sa_prefix+"-kick"+sa_postfix);
  }
  else if ((m_wants_buttjump || m_does_buttjump) && is_big()) {
    m_sprite->set_action(sa_prefix+"-buttjump"+sa_postfix, 1);
  }
  else if (!on_ground() || m_fall_mode != ON_GROUND) {
    if(m_physic.get_velocity_x() != 0 || m_fall_mode != ON_GROUND) {
        m_sprite->set_action(sa_prefix+"-jump"+sa_postfix);
    }
  }
  else {
    if (fabsf(m_physic.get_velocity_x()) < 1.0f) {
      // Determine which idle stage we're at
      if (m_sprite->get_action().find("-stand-") == std::string::npos && m_sprite->get_action().find("-idle-") == std::string::npos) {
        m_idle_stage = 0;
        m_idle_timer.start(static_cast<float>(IDLE_TIME[m_idle_stage]) / 1000.0f);

        m_sprite->set_action_continued(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix);
      }
      else if (m_idle_timer.check() || (IDLE_TIME[m_idle_stage] == 0 && m_sprite->animation_done())) {
        m_idle_stage++;
        if (m_idle_stage >= IDLE_STAGE_COUNT)
          m_idle_stage = 1;

        m_idle_timer.start(static_cast<float>(IDLE_TIME[m_idle_stage]) / 1000.0f);

        if (IDLE_TIME[m_idle_stage] == 0)
          m_sprite->set_action(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix, 1);
        else
          m_sprite->set_action(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix);
      }
      else {
        m_sprite->set_action_continued(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix);
      }
    }
    else {
      if(fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM && !is_big()) {
        m_sprite->set_action(sa_prefix+"-run"+sa_postfix);
      } else {
        m_sprite->set_action(sa_prefix+"-walk"+sa_postfix);
      }
    }
  }

  /* Set Tux powerup sprite action */
  if (m_player_status->bonus == EARTH_BONUS) {
    m_powersprite->set_action(m_sprite->get_action());
    m_lightsprite->set_action(m_sprite->get_action());
  } else if (m_player_status->bonus == AIR_BONUS) {
    m_powersprite->set_action(m_sprite->get_action());
  } else if (m_player_status->bonus == FIRE_BONUS && g_config->christmas_mode) {
    m_powersprite->set_action(m_sprite->get_action());
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
  if (m_safe_timer.started() && size_t(game_time*40)%2)
    ;  // don't draw Tux
  else if (m_player_status->bonus == EARTH_BONUS){ // draw special effects with earthflower bonus
    // shake at end of maximum stone duration
    Vector shake_delta = (m_stone && m_ability_timer.get_timeleft() < 1.0f) ? Vector(graphicsRandom.randf(-3.0f, 3.0f) * 1.0f, 0) : Vector(0,0);
    m_sprite->draw(context.color(), get_pos() + shake_delta, LAYER_OBJECTS + 1);
    // draw hardhat
    m_powersprite->draw(context.color(), get_pos() + shake_delta, LAYER_OBJECTS + 1);
    // light
    m_lightsprite->draw(context.light(), get_pos(), 0);

    // give an indicator that stone form cannot be used for a while
    if (m_cooldown_timer.started() && graphicsRandom.rand(0, 4) == 0) {
      float px = graphicsRandom.randf(bbox.p1.x, bbox.p2.x);
      float py = bbox.p2.y+8;
      Vector ppos = Vector(px, py);
      Sector::current()->add_object(std::make_shared<SpriteParticle>(
        "images/objects/particles/sparkle.sprite", "dark",
        ppos, ANCHOR_MIDDLE, Vector(0, 0), Vector(0, 0), LAYER_OBJECTS+1+5));
    }
  }
  else {
    if(m_dying)
      m_sprite->draw(context.color(), get_pos(), Sector::current()->get_foremost_layer());
    else
      m_sprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);

    if (m_player_status->bonus == AIR_BONUS)
      m_powersprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);
    else if(m_player_status->bonus == FIRE_BONUS && g_config->christmas_mode) {
      m_powersprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);
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
    m_ice_this_frame = true;
    m_on_ice = true;
  }
}

void
Player::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom) {
    if(m_physic.get_velocity_y() > 0)
      m_physic.set_velocity_y(0);

    m_on_ground_flag = true;
    m_floor_normal = hit.slope_normal;

    // Butt Jump landed
    if (m_does_buttjump) {
      m_does_buttjump = false;
      m_physic.set_velocity_y(-300);
      m_on_ground_flag = false;
      Sector::current()->add_object(std::make_shared<Particles>(
                                      bbox.p2,
                                      50, 70, 260, 280, Vector(0, 300), 3,
                                      Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1));
      Sector::current()->add_object(std::make_shared<Particles>(
                                      Vector(bbox.p1.x, bbox.p2.y),
                                      -70, -50, 260, 280, Vector(0, 300), 3,
                                      Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1));
      camera->shake(.1f, 0, 5);
    }

  } else if(hit.top) {
    if(m_physic.get_velocity_y() < 0)
      m_physic.set_velocity_y(.2f);
  }

  if((hit.left || hit.right) && hit.slope_normal.x == 0) {
    m_physic.set_velocity_x(0);
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
  auto bullet = dynamic_cast<Bullet*> (&other);
  if(bullet) {
    return FORCE_MOVE;
  }

  auto player = dynamic_cast<Player*> (&other);
  if(player) {
    return ABORT_MOVE;
  }

  if(hit.left || hit.right) {
    try_grab(); //grab objects right now, in update it will be too late
  }
  assert(dynamic_cast<MovingObject*> (&other) != NULL);
  auto moving_object = static_cast<MovingObject*> (&other);
  if(moving_object->get_group() == COLGROUP_TOUCHABLE) {
    auto trigger = dynamic_cast<TriggerBase*> (&other);
    if(trigger && !m_deactivated) {
      if(m_controller->pressed(Controller::UP))
        trigger->event(*this, TriggerBase::EVENT_ACTIVATE);
    }

    return FORCE_MOVE;
  }

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy != NULL) {
    if(m_safe_timer.started() || m_invincible_timer.started())
      return FORCE_MOVE;
    if(m_stone)
      return ABORT_MOVE;
  }

  return CONTINUE;
}

void
Player::make_invincible()
{
  SoundManager::current()->play("sounds/invincible_start.ogg");
  m_invincible_timer.start(TUX_INVINCIBLE_TIME);
  Sector::current()->play_music(HERRING_MUSIC);
}

/* Kill Player! */
void
Player::kill(bool completely)
{
  if(m_dying || m_deactivated || is_winning() )
    return;

  if(!completely && (m_safe_timer.started() || m_invincible_timer.started() || m_stone))
    return;

  m_growing = false;

  if (m_climbing) stop_climbing(*m_climbing);

  m_physic.set_velocity_x(0);

  m_sprite->set_angle(0.0f);
  m_powersprite->set_angle(0.0f);
  m_lightsprite->set_angle(0.0f);

  if(!completely && is_big()) {
    SoundManager::current()->play("sounds/hurt.wav");

    if(m_player_status->bonus == FIRE_BONUS
      || m_player_status->bonus == ICE_BONUS
      || m_player_status->bonus == AIR_BONUS
      || m_player_status->bonus == EARTH_BONUS) {
      m_safe_timer.start(TUX_SAFE_TIME);
      set_bonus(GROWUP_BONUS, true);
    } else if(m_player_status->bonus == GROWUP_BONUS) {
      m_safe_timer.start(TUX_SAFE_TIME /* + GROWING_TIME */);
      m_duck = false;
      stop_backflipping();
      set_bonus(NO_BONUS, true);
    }
  } else {
    SoundManager::current()->play("sounds/kill.wav");

    // do not die when in edit mode
    if (m_edit_mode) {
      set_ghost_mode(true);
      return;
    }

    if (m_player_status->coins >= 25 && !GameSession::current()->get_reset_point_sectorname().empty())
    {
      for (int i = 0; i < 5; i++)
      {
        // the numbers: starting x, starting y, velocity y
        Sector::current()->add_object(std::make_shared<FallingCoin>(get_pos() +
                                                      Vector(graphicsRandom.randf(5.0f), graphicsRandom.randf(-32.0f, 18.0f)),
                                                      graphicsRandom.randf(-100.0f, 100.0f)));
      }
      m_player_status->coins -= std::max(m_player_status->coins/10, 25);
    }
    else
    {
      GameSession::current()->set_reset_point("", Vector());
    }
    m_physic.enable_gravity(true);
    m_physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
    m_safe_timer.stop();
    m_invincible_timer.stop();
    m_physic.set_acceleration(0, 0);
    m_physic.set_velocity(0, -700);
    set_bonus(NO_BONUS, true);
    m_dying = true;
    m_dying_timer.start(3.0);
    set_group(COLGROUP_DISABLED);

    // TODO: need nice way to handle players dying in co-op mode
    Sector::current()->m_effect->fade_out(3.0);
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
  m_duck = false;
  stop_backflipping();
  m_last_ground_y = vector.y;
  if (m_climbing) stop_climbing(*m_climbing);

  m_physic.reset();
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
  if ((get_pos().y > Sector::current()->get_height()) && (!m_ghost_mode)) {
    kill(true);
    return;
  }
}

void
Player::add_velocity(const Vector& velocity)
{
  m_physic.set_velocity(m_physic.get_velocity() + velocity);
}

void
Player::add_velocity(const Vector& velocity, const Vector& end_speed)
{
  if (end_speed.x > 0)
    m_physic.set_velocity_x(std::min(m_physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.x < 0)
    m_physic.set_velocity_x(std::max(m_physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.y > 0)
    m_physic.set_velocity_y(std::min(m_physic.get_velocity_y() + velocity.y, end_speed.y));
  if (end_speed.y < 0)
    m_physic.set_velocity_y(std::max(m_physic.get_velocity_y() + velocity.y, end_speed.y));
}

Vector
Player::get_velocity() const
{
  return m_physic.get_velocity();
}

void
Player::bounce(BadGuy& )
{
  if(!(m_player_status->bonus == AIR_BONUS))
    m_physic.set_velocity_y(m_controller->hold(Controller::JUMP) ? -520 : -300);
  else {
    m_physic.set_velocity_y(m_controller->hold(Controller::JUMP) ? -580 : -340);
    m_ability_time = static_cast<float>(m_player_status->max_air_time) * GLIDE_TIME_PER_FLOWER;
  }
}

//scripting Functions Below

void
Player::deactivate()
{
  if (m_deactivated)
    return;
  m_deactivated = true;
  m_physic.set_velocity_x(0);
  m_physic.set_velocity_y(0);
  m_physic.set_acceleration_x(0);
  m_physic.set_acceleration_y(0);
  if (m_climbing) stop_climbing(*m_climbing);
}

void
Player::activate()
{
  if (!m_deactivated)
    return;
  m_deactivated = false;
}

void Player::walk(float speed)
{
  m_physic.set_velocity_x(speed);
}

void Player::set_dir(bool right)
{
  m_dir = right ? RIGHT : LEFT;
}

void
Player::set_ghost_mode(bool enable)
{
  if (m_ghost_mode == enable)
    return;

  if (m_climbing) stop_climbing(*m_climbing);

  if (m_grabbed_object) {
    m_grabbed_object->ungrab(*this, m_dir);
    m_grabbed_object = NULL;
  }

  if (enable) {
    m_ghost_mode = true;
    set_group(COLGROUP_DISABLED);
    m_physic.enable_gravity(false);
    log_debug << "You feel lightheaded. Use movement controls to float around, press ACTION to scare badguys." << std::endl;
  } else {
    m_ghost_mode = false;
    set_group(COLGROUP_MOVING);
    m_physic.enable_gravity(true);
    log_debug << "You feel solid again." << std::endl;
  }
}

void
Player::set_edit_mode(bool enable)
{
  m_edit_mode = enable;
}

void
Player::start_climbing(Climbable& climbable)
{
  if (m_climbing) return;

  m_climbing = &climbable;
  m_physic.enable_gravity(false);
  m_physic.set_velocity(0, 0);
  m_physic.set_acceleration(0, 0);
  if (m_backflipping) {
    stop_backflipping();
    do_standup();
  }
}

void
Player::stop_climbing(Climbable& /*climbable*/)
{
  if (!m_climbing) return;

  m_climbing = 0;

  if (m_grabbed_object) {
    m_grabbed_object->ungrab(*this, m_dir);
    m_grabbed_object = NULL;
  }

  m_physic.enable_gravity(true);
  m_physic.set_velocity(0, 0);
  m_physic.set_acceleration(0, 0);

  if ((m_controller->hold(Controller::JUMP)) || (m_controller->hold(Controller::UP))) {
    m_on_ground_flag = true;
    // TODO: This won't help. Why?
    do_jump(-300);
  }
}

void
Player::handle_input_climbing()
{
  if (!m_climbing) {
    log_warning << "handle_input_climbing called with climbing set to 0. Input handling skipped" << std::endl;
    return;
  }

  float vx = 0;
  float vy = 0;
  if (m_controller->hold(Controller::LEFT)) {
    m_dir = LEFT;
    vx -= MAX_CLIMB_XM;
  }
  if (m_controller->hold(Controller::RIGHT)) {
    m_dir = RIGHT;
    vx += MAX_CLIMB_XM;
  }
  if (m_controller->hold(Controller::UP)) {
    vy -= MAX_CLIMB_YM;
  }
  if (m_controller->hold(Controller::DOWN)) {
    vy += MAX_CLIMB_YM;
  }
  if (m_controller->hold(Controller::JUMP)) {
    if (m_can_jump) {
      stop_climbing(*m_climbing);
      return;
    }
  } else {
    m_can_jump = true;
  }
  if (m_controller->hold(Controller::ACTION)) {
    stop_climbing(*m_climbing);
    return;
  }
  m_physic.set_velocity(vx, vy);
  m_physic.set_acceleration(0, 0);
}

void
Player::stop_backflipping()
{
  m_backflipping = false;
  m_backflip_direction = 0;
  m_sprite->set_angle(0.0f);
  m_powersprite->set_angle(0.0f);
  m_lightsprite->set_angle(0.0f);
}

bool
Player::has_grabbed(const std::string& object_name) const
{
  if(object_name.empty())
  {
    return false;
  }
  if(auto object = dynamic_cast<GameObject*>(m_grabbed_object))
  {
    return object->get_name() == object_name;
  }
  return false;
}

/* EOF */
