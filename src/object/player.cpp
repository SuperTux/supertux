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
#include "math/util.hpp"
#include "math/random.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/falling_coin.hpp"
#include "object/music_object.hpp"
#include "object/particles.hpp"
#include "object/portable.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "trigger/climbable.hpp"
#include "trigger/trigger_base.hpp"
#include "video/surface.hpp"

#define SWIMMING

const float TUX_INVINCIBLE_TIME_WARNING = 2.0f;

namespace {

/* Times: */
const float TUX_SAFE_TIME = 1.8f;
const float TUX_INVINCIBLE_TIME = 14.0f;
const float TUX_BACKFLIP_TIME = 2.1f; // minimum air time that backflip results in a loss of control

const float BUTTJUMP_MIN_VELOCITY_Y = 400.0f;
const float SHOOTING_TIME = .150f;
const float GLIDE_TIME_PER_FLOWER = 0.5f;
const float STONE_TIME_PER_FLOWER = 2.0f;

/** number of idle stages, including standing */
const unsigned int IDLE_STAGE_COUNT = 5;
/**
 * how long to play each idle animation in milliseconds
 * '0' means the sprite action is played once before moving onto the next
 * animation
 */
const int IDLE_TIME[] = { 5000, 0, 2500, 0, 2500 };
/** idle stages */
const std::string IDLE_STAGES[] =
{ "stand",
  "idle",
  "stand",
  "idle",
  "stand" };

/** acceleration in horizontal direction when walking
 * (all accelerations are in  pixel/s^2) */
const float WALK_ACCELERATION_X = 300;
/** acceleration in horizontal direction when running */
const float RUN_ACCELERATION_X = 400;
/** acceleration when skidding */
const float SKID_XM = 200;
/** time of skidding in seconds */
const float SKID_TIME = .3f;
/** maximum walk velocity (pixel/s) */
const float MAX_WALK_XM = 230;
/** maximum run velocity (pixel/s) */
const float MAX_RUN_XM = 320;
/** bonus run velocity addition (pixel/s) */
const float BONUS_RUN_XM = 80;
/** maximum horizontal climb velocity */
const float MAX_CLIMB_XM = 96;
/** maximum vertical climb velocity */
const float MAX_CLIMB_YM = 128;
/** maximum vertical glide velocity */
const float MAX_GLIDE_YM = 128;
/** sliding down walls velocity */
const float MAX_WALLCLING_YM = 64;
/** instant velocity when tux starts to walk */
const float WALK_SPEED = 100;
/** rate at which m_boost decreases */
const float BOOST_DECREASE_RATE = 500;
/** rate at which the speed decreases if going above maximum */
const float OVERSPEED_DECELERATION = 100;

/** multiplied by WALK_ACCELERATION to give friction */
const float NORMAL_FRICTION_MULTIPLIER = 1.5f;
/** multiplied by WALK_ACCELERATION to give friction */
const float ICE_FRICTION_MULTIPLIER = 0.1f;
const float ICE_ACCELERATION_MULTIPLIER = 0.25f;

/** time of the kick (kicking mriceblock) animation */
const float KICK_TIME = .3f;

/** if Tux cannot unduck for this long, he will get hurt */
const float UNDUCK_HURT_TIME = 0.25f;
/** gravity is higher after the jump key is released before
    the apex of the jump is reached */
const float JUMP_EARLY_APEX_FACTOR = 3.0;

const float JUMP_GRACE_TIME = 0.25f; /**< time before hitting the ground that the jump button may be pressed (and still trigger a jump) */
const float COYOTE_TIME = 0.1f; /**< time between the moment leaving a platform without jumping and being able to jump anyways despite being in the air */

/* Tux's collision rectangle */
const float TUX_WIDTH = 31.8f;
const float RUNNING_TUX_WIDTH = 34;
const float SMALL_TUX_HEIGHT = 30.8f;
const float BIG_TUX_HEIGHT = 62.8f;
const float DUCKED_TUX_HEIGHT = 31.8f;

/** when Tux swims down and approaches the bottom of the screen, push him back up with that strength */
const float WATER_FALLOUT_FORCEBACK_STRENGTH = 1024.f;

bool no_water = true;

} // namespace

Player::Player(PlayerStatus& player_status, const std::string& name_) :
  ExposedObject<Player, scripting::Player>(this),
  m_deactivated(false),
  m_controller(&InputManager::current()->get_controller()),
  m_scripting_controller(new CodeController()),
  m_player_status(player_status),
  m_duck(false),
  m_dead(false),
  m_dying(false),
  m_winning(false),
  m_backflipping(false),
  m_backflip_direction(0),
  m_peekingX(Direction::AUTO),
  m_peekingY(Direction::AUTO),
  m_ability_time(),
  m_stone(false),
  m_falling_below_water(false),
  m_swimming(false),
  m_swimboosting(false),
  m_on_left_wall(false),
  m_on_right_wall(false),
  m_in_walljump_tile(false),
  m_can_walljump(false),
  m_boost(0.f),
  m_speedlimit(0), //no special limit
  m_scripting_controller_old(nullptr),
  m_jump_early_apex(false),
  m_on_ice(false),
  m_ice_this_frame(false),
  m_lightsprite(SpriteManager::current()->create("images/creatures/tux/light.sprite")),
  m_powersprite(SpriteManager::current()->create("images/creatures/tux/powerups.sprite")),
  m_dir(Direction::RIGHT),
  m_old_dir(m_dir),
  m_last_ground_y(0),
  m_fall_mode(ON_GROUND),
  m_on_ground_flag(false),
  m_jumping(false),
  m_can_jump(true),
  m_jump_button_timer(),
  m_coyote_timer(),
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
  m_grabbed_object(nullptr),
  m_grabbed_object_remove_listener(new GrabListener(*this)),
  // if/when we have complete penny gfx, we can
  // load those instead of Tux's sprite in the
  // constructor
  m_sprite(SpriteManager::current()->create("images/creatures/tux/tux.sprite")),
  m_swimming_angle(0),
  m_swimming_accel_modifier(100.f),
  m_water_jump(false),
  m_airarrow(Surface::from_file("images/engine/hud/airarrow.png")),
  m_floor_normal(0.0f, 0.0f),
  m_ghost_mode(false),
  m_edit_mode(false),
  m_unduck_hurt_timer(),
  m_idle_timer(),
  m_idle_stage(0),
  m_climbing(nullptr),
  m_climbing_remove_listener(nullptr)
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
  m_col.set_size(TUX_WIDTH, is_big() ? BIG_TUX_HEIGHT : SMALL_TUX_HEIGHT);

  m_sprite->set_angle(0.0f);
  m_powersprite->set_angle(0.0f);
  m_lightsprite->set_angle(0.0f);
  m_lightsprite->set_blend(Blend::ADD);

  m_physic.reset();
}

Player::~Player()
{
  ungrab_object();
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
Player::set_controller(const Controller* controller_)
{
  m_controller = controller_;
}

void
Player::set_winning()
{
  if ( ! is_winning() ){
    m_winning = true;
    m_invincible_timer.start(10000.0f);
  }
}

void
Player::use_scripting_controller(bool use_or_release)
{
  if ((use_or_release == true) && (m_controller != m_scripting_controller.get())) {
    m_scripting_controller_old = &get_controller();
    set_controller(m_scripting_controller.get());
  }
  if ((use_or_release == false) && (m_controller == m_scripting_controller.get())) {
    set_controller(m_scripting_controller_old);
    m_scripting_controller_old = nullptr;
  }
}

void
Player::do_scripting_controller(const std::string& control_text, bool pressed)
{
  if (const auto maybe_control = Control_from_string(control_text)) {
    m_scripting_controller->press(*maybe_control, pressed);
  }
}

bool
Player::adjust_height(float new_height, float bottom_offset)
{
  Rectf bbox2 = m_col.m_bbox;
  bbox2.move(Vector(0, m_col.m_bbox.get_height() - new_height - bottom_offset));
  bbox2.set_height(new_height);


  if (new_height > m_col.m_bbox.get_height()) {
    //Rectf additional_space = bbox2;
    //additional_space.set_height(new_height - m_col.m_bbox.get_height());
    if (!Sector::get().is_free_of_statics(bbox2, this, true))
      return false;
  }

  // adjust bbox accordingly
  // note that we use members of moving_object for this, so we can run this during CD, too
  set_pos(bbox2.p1());
  m_col.set_size(bbox2.get_width(), bbox2.get_height());
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
Player::update(float dt_sec)
{
  check_bounds();

  //handling of swimming

#ifdef SWIMMING
  if (no_water)
  {
    if (m_swimming)
    {
      m_water_jump = true;
      if (m_physic.get_velocity_y() > -350.f && m_controller->hold(Control::UP))
        m_physic.set_velocity_y(-350.f);
    }
    m_swimming = false;
  }

  if ((on_ground() || m_climbing || m_does_buttjump) && m_water_jump)
  {
    if (is_big() && !adjust_height(BIG_TUX_HEIGHT))
    {
      //Force Tux's box up a little in order to not phase into floor
      adjust_height(BIG_TUX_HEIGHT, 10.f);
      do_duck();
    }
    else if (!is_big())
    {
      adjust_height(SMALL_TUX_HEIGHT);
    }
    m_dir = (m_physic.get_velocity_x() >= 0.f) ? Direction::RIGHT : Direction::LEFT;
    m_water_jump = false;
    m_swimboosting = false;
    m_powersprite->set_angle(0.f);
    m_lightsprite->set_angle(0.f);
  }
  no_water = true;

  if ((m_swimming || m_water_jump) && is_big())
  {
    m_col.set_size(TUX_WIDTH, TUX_WIDTH);
    adjust_height(TUX_WIDTH);
  }

  Rectf swim_here_box = get_bbox();
  swim_here_box.set_bottom(m_col.m_bbox.get_bottom() - 16.f);
  bool can_swim_here = !Sector::get().is_free_of_tiles(swim_here_box, true, Tile::WATER);

  if (m_swimming)
  {
    if (can_swim_here)
    {
      no_water = false;
    }
    else
    {
      m_swimming = false;
      m_water_jump = true;
      if (m_physic.get_velocity_y() > -350.f && m_controller->hold(Control::UP))
        m_physic.set_velocity_y(-350.f);
    }
  }
  else
  {
    if (can_swim_here && !m_stone && !m_climbing)
    {
      no_water = false;
      m_water_jump = false;
      m_swimming = true;
      m_swimming_angle = math::angle(Vector(m_physic.get_velocity_x(), m_physic.get_velocity_y()));
      if (is_big())
        adjust_height(TUX_WIDTH);
      m_wants_buttjump = m_does_buttjump = m_backflipping = false;
      m_dir = (m_physic.get_velocity_x() > 0) ? Direction::LEFT : Direction::RIGHT;
      SoundManager::current()->play("sounds/splash.wav");
    }
  }
#endif

  //end of swimming handling

  if (m_dying && m_dying_timer.check()) {
    Sector::get().stop_looping_sounds();
    set_bonus(NO_BONUS, true);
    m_dead = true;
    return;
  }

  if (!m_dying && !m_deactivated)
    handle_input();

  /*
  // handle_input() calls apply_friction() when Tux is not walking, so we'll have to do this ourselves
  if (deactivated)
  apply_friction();
  */

  // extend/shrink tux collision rectangle so that we fall through/walk over 1
  // tile holes

  //wallclinging and walljumping

  Rectf wallclingleft = get_bbox();
  wallclingleft.set_left(wallclingleft.get_left() - 8.f);
  m_on_left_wall = !Sector::get().is_free_of_statics(wallclingleft);

  Rectf wallclingright = get_bbox();
  wallclingright.set_right(wallclingright.get_right() + 8.f);
  m_on_right_wall = !Sector::get().is_free_of_statics(wallclingright);

  m_can_walljump = ((m_on_right_wall || m_on_left_wall) && !on_ground() && !m_swimming && m_in_walljump_tile);
  if (m_can_walljump && (m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT)) && m_physic.get_velocity_y() >= 0.f && !m_controller->pressed(Control::JUMP))
  {
    m_physic.set_velocity_y(MAX_WALLCLING_YM);
    m_physic.set_acceleration_y(0);
    if (m_water_jump)
    {
      adjust_height(is_big() ? BIG_TUX_HEIGHT : SMALL_TUX_HEIGHT);
      m_water_jump = false;
    }
    m_powersprite->set_angle(0.f);
    m_lightsprite->set_angle(0.f);
  }

  m_in_walljump_tile = false;

  //End of wallclinging

    // extend/shrink tux collision rectangle so that we fall through/walk over 1
    // tile holes
    if (fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) {
      m_col.set_width(RUNNING_TUX_WIDTH);
    } else {
      m_col.set_width(TUX_WIDTH);
    }

  // on downward slopes, adjust vertical velocity so tux walks smoothly down
  if (on_ground() && !m_swimming && !m_dying) {
    if (m_floor_normal.y != 0) {
      if ((m_floor_normal.x * m_physic.get_velocity_x()) >= 0) {
        m_physic.set_velocity_y(250);
      }
    }
  }

  // handle backflipping
  if (m_backflipping && !m_dying) {
    //prevent player from changing direction when backflipping
    m_dir = (m_backflip_direction == 1) ? Direction::LEFT : Direction::RIGHT;
    if (m_backflip_timer.started()) m_physic.set_velocity_x(100.0f * static_cast<float>(m_backflip_direction));
    //rotate sprite during flip
    m_sprite->set_angle(m_sprite->get_angle() + (m_dir == Direction::LEFT ? 1 : -1) * dt_sec * (360.0f / 0.5f));
    if (m_player_status.has_hat_sprite() && !m_swimming && !m_water_jump)
      m_powersprite->set_angle(m_sprite->get_angle());
    if (m_player_status.bonus == EARTH_BONUS)
      m_lightsprite->set_angle(m_sprite->get_angle());
  }

  if (on_ground()) {
    m_coyote_timer.start(COYOTE_TIME);
  }

  // set fall mode...
  if (on_ground()) {
    m_fall_mode = ON_GROUND;
    m_last_ground_y = get_pos().y;
  } else {
    if (get_pos().y > m_last_ground_y)
      m_fall_mode = FALLING;
    else if (m_fall_mode == ON_GROUND)
      m_fall_mode = JUMPING;
  }

  // check if we landed
  if (on_ground()) {
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
        do_standup(false);
    }
    if (m_player_status.bonus == AIR_BONUS)
      m_ability_time = static_cast<float>(m_player_status.max_air_time) * GLIDE_TIME_PER_FLOWER;
  }

  if (m_second_growup_sound_timer.check())
  {
    SoundManager::current()->play("sounds/grow.wav");
    m_second_growup_sound_timer.stop();
  }

  // Handle player approaching the bottom of the screen while swimming
  if (m_falling_below_water) {
    m_physic.set_velocity_y(std::min(m_physic.get_velocity_y(), 0.f));
  }

  if ((get_pos().y > Sector::get().get_height() - m_col.m_bbox.get_height()) && (!m_ghost_mode && m_swimming))
  {
    m_physic.set_acceleration_y(-WATER_FALLOUT_FORCEBACK_STRENGTH);
  }

  if (m_boost != 0.f)
  {
    bool sign = std::signbit(m_boost);
    m_boost = (sign ? -1.f : +1.f) * (std::abs(m_boost) - dt_sec * BOOST_DECREASE_RATE);
    if (std::signbit(m_boost) != sign)
      m_boost = 0.f;
  }

  // calculate movement for this frame
  m_col.set_movement(m_physic.get_movement(dt_sec) + Vector(m_boost * dt_sec, 0));

  if (m_grabbed_object != nullptr && !m_dying)
  {
    position_grabbed_object();
  }

  if (m_dying)
    ungrab_object();

  if (!m_ice_this_frame && on_ground())
    m_on_ice = false;

  m_on_ground_flag = false;
  m_ice_this_frame = false;

  // when invincible, spawn particles
  if (m_invincible_timer.started())
  {
    if (graphicsRandom.rand(0, 2) == 0)
    {
      float px = graphicsRandom.randf(m_col.m_bbox.get_left()+0, m_col.m_bbox.get_right()-0);
      float py = graphicsRandom.randf(m_col.m_bbox.get_top()+0, m_col.m_bbox.get_bottom()-0);
      Vector ppos = Vector(px, py);
      Vector pspeed = Vector(0, 0);
      Vector paccel = Vector(0, 0);
      Sector::get().add<SpriteParticle>(
        "images/particles/sparkle.sprite",
        // draw bright sparkle when there is lots of time left,
        // dark sparkle when invincibility is about to end
        (m_invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING) ?
        // make every other a longer sparkle to make trail a bit fuzzy
        (size_t(g_game_time*20)%2) ? "small" : "medium"
        :
        "dark", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS + 1 + 5);
    }
  }

  if (m_growing) {
    if (m_sprite->animation_done()) m_growing = false;
  }

  // when climbing animate only while moving
  if (m_climbing){
    if ((m_physic.get_velocity_x() == 0) && (m_physic.get_velocity_y() == 0))
    {
      m_sprite->stop_animation();
      m_powersprite->stop_animation();
    }
    else
    {
      m_sprite->set_animation_loops(-1);
      m_powersprite->set_animation_loops(-1);
    }
  }

}

void
Player::handle_input_swimming()
{
  float pointx = float(m_controller->hold(Control::RIGHT)) - float(m_controller->hold(Control::LEFT)),
        pointy = float(m_controller->hold(Control::DOWN)) - float(m_controller->hold(Control::UP));

  bool boost = m_controller->hold(Control::JUMP);

  swim(pointx,pointy,boost);
}

void
Player::swim(float pointx, float pointy, bool boost)
{
    if (m_swimming)
      m_physic.set_gravity_modifier(.0f);

    // Angle
    bool is_ang_defined = (pointx != 0) || (pointy != 0);
    float pointed_angle = math::angle(Vector(pointx, pointy));
    float delta = 0;

    if(is_ang_defined)
    {
      delta = pointed_angle - m_swimming_angle;
  
      if(std::abs(delta) > math::PI)
        delta += delta > 0 ? -math::TAU : math::TAU;

      float epsilon = (boost ? .3f : .15f) * delta;
      m_swimming_angle += epsilon;

      if (m_swimming_angle > math::PI)
        m_swimming_angle -= math::TAU;

      if (m_swimming_angle <= -math::PI)
        m_swimming_angle += math::TAU;
    }

    float vx = m_physic.get_velocity_x();
    float vy = m_physic.get_velocity_y();

    if (m_swimming && !m_water_jump)
    {

      if(is_ang_defined && std::abs(delta) < 0.01f)
        m_swimming_angle = pointed_angle;

      m_swimming_accel_modifier = is_ang_defined ? 600.f : 0.f;
      Vector swimming_direction = math::vec2_from_polar(m_swimming_accel_modifier, pointed_angle);

      m_physic.set_acceleration_x((swimming_direction.x - 1.0f * vx) * 2.f);
      m_physic.set_acceleration_y((swimming_direction.y - 1.0f * vy) * 2.f);

      // Limit speed, if you go above this speed your acceleration is set to opposite (?)
      float limit = 300.f;
      if (glm::length(m_physic.get_velocity()) > limit)
      {
        m_physic.set_acceleration(-vx,-vy);   // Was too lazy to set it properly ~~zwatotem
      }

      // Natural friction
      if (!is_ang_defined)
      {
        m_physic.set_acceleration(-3.f*vx, -3.f*vy);
      }

      //not boosting? let's slow this penguin down!!!
      if (!boost && is_ang_defined && glm::length(m_physic.get_velocity()) > 310.f)
      {
        m_physic.set_acceleration(-5.f*vx, -5.f*vy);
      }

      // Snapping to prevent unwanted floating
        if (!is_ang_defined && glm::length(Vector(vx,vy)) < 100.f)
      {
        vx = 0;
        vy = 0;
      }

      // Turbo, using pointsign
      float minboostspeed = 100.f;
      if (boost && glm::length(m_physic.get_velocity()) > minboostspeed)
      {
        if (glm::length(m_physic.get_velocity()) < 600.f)
        {
          m_swimboosting = true;
          if (is_ang_defined)
          {
            vx += 30.f * pointx;
            vy += 30.f * pointy;
          }
        }
        else
        {
          //cap on boosting
          m_physic.set_acceleration(-vx, -vy);
        }
        m_physic.set_velocity(vx, vy);
      }
      else
      {
          if (glm::length(m_physic.get_velocity()) < 310.f)
        {
          m_swimboosting = false;
        }
      }
    }
    if (m_water_jump && !m_swimming)
    {
      m_swimming_angle = math::angle(Vector(vx, vy));
    }

  // snap angle dir when water jumping to avoid crazy spinning graphics...
  if (m_water_jump && !m_swimming && std::abs(m_physic.get_velocity_x()) < 10.f)
  {
    m_sprite->set_angle(math::degrees(m_swimming_angle));
    m_powersprite->set_angle(math::degrees(m_swimming_angle));
    if (m_lightsprite)
    {
      m_lightsprite->set_angle(math::degrees(m_swimming_angle));
    }
  }
  else
  {
    // otherwise angle the sprite normally
    float angle = (std::abs(m_swimming_angle) <= math::PI_2) ?
                    math::degrees(m_swimming_angle) :
                    math::degrees(math::PI + m_swimming_angle);

    m_sprite->set_angle(angle);
    m_powersprite->set_angle(angle);
    if (m_lightsprite)
    {
      m_lightsprite->set_angle(angle);
    }

    //Force the speed to point in the direction Tux is going
    if (m_swimming && !m_water_jump && boost)
    {
      m_physic.set_velocity(math::at_angle(m_physic.get_velocity(), m_swimming_angle));
    }
  }
}

bool
Player::on_ground() const
{
  return m_on_ground_flag;
}

void
Player::set_on_ground(bool flag)
{
  m_on_ground_flag = flag;
}

bool
Player::is_big() const
{
  if (m_player_status.bonus == NO_BONUS)
    return false;

  return true;
}

void
Player::apply_friction()
{
  bool is_on_ground = on_ground();
  float velx = m_physic.get_velocity_x();
  if (is_on_ground && fabsf(velx) < WALK_SPEED) {
    m_physic.set_velocity_x(0);
    m_physic.set_acceleration_x(0);
    return;
  }
  float friction = WALK_ACCELERATION_X;
  if (m_on_ice && is_on_ground)
    friction *= ICE_FRICTION_MULTIPLIER;
  else
    friction *= NORMAL_FRICTION_MULTIPLIER;
  if (velx < 0) {
    m_physic.set_acceleration_x(friction);
  } else if (velx > 0) {
    m_physic.set_acceleration_x(-friction);
  } // no friction for physic.get_velocity_x() == 0
}

void
Player::handle_horizontal_input()
{
  float vx = m_physic.get_velocity_x();
  float vy = m_physic.get_velocity_y();
  float ax = m_physic.get_acceleration_x(); // NOLINT
  float ay = m_physic.get_acceleration_y();

  float dirsign = 0;
  if (!m_duck || m_physic.get_velocity_y() != 0) {
    if (m_controller->hold(Control::LEFT) && !m_controller->hold(Control::RIGHT)) {
      m_old_dir = m_dir;
      if (!m_water_jump) m_dir = Direction::LEFT;
      dirsign = -1;
    } else if (!m_controller->hold(Control::LEFT)
              && m_controller->hold(Control::RIGHT)) {
      m_old_dir = m_dir;
      if (!m_water_jump) m_dir = Direction::RIGHT;
      dirsign = 1;
    }
  }

  // do not run if we're holding something which slows us down
  if ( m_grabbed_object && m_grabbed_object->is_hampering() ) {
    ax = dirsign * WALK_ACCELERATION_X;
    // limit speed
    if (vx >= MAX_WALK_XM && dirsign > 0) {
      ax = std::min(ax, -OVERSPEED_DECELERATION);
    } else if (vx <= -MAX_WALK_XM && dirsign < 0) {
      ax = std::max(ax, OVERSPEED_DECELERATION);
    }
  } else {
    if ( vx * dirsign < MAX_WALK_XM ) {
      ax = dirsign * WALK_ACCELERATION_X;
    } else {
      ax = dirsign * RUN_ACCELERATION_X;
    }
    // limit speed
    if (vx >= MAX_RUN_XM + BONUS_RUN_XM *((m_player_status.bonus == AIR_BONUS) ? 1 : 0)) {
      ax = std::min(ax, -OVERSPEED_DECELERATION);
    } else if (vx <= -MAX_RUN_XM - BONUS_RUN_XM *((m_player_status.bonus == AIR_BONUS) ? 1 : 0)) {
      ax = std::max(ax, OVERSPEED_DECELERATION);
    }
  }

  // we can reach WALK_SPEED without any acceleration
  if (dirsign != 0 && fabsf(vx) < WALK_SPEED) {
    vx = dirsign * WALK_SPEED;
  }

  //Check speedlimit.
  if ( m_speedlimit > 0 &&  vx * dirsign >= m_speedlimit ) {
    vx = dirsign * m_speedlimit;
    ax = 0;
  }

  // changing directions?
  if ((vx < 0 && dirsign >0) || (vx>0 && dirsign<0)) {
    if (on_ground()) {
      // let's skid!
      if (fabsf(vx)>SKID_XM && !m_skidding_timer.started()) {
        m_skidding_timer.start(SKID_TIME);
        SoundManager::current()->play("sounds/skid.wav");
        // dust some particles
        Sector::get().add<Particles>(
            Vector(m_dir == Direction::LEFT ? m_col.m_bbox.get_right() : m_col.m_bbox.get_left(), m_col.m_bbox.get_bottom()),
            m_dir == Direction::LEFT ? 50 : -70, m_dir == Direction::LEFT ? 70 : -50, 260.0f, 280.0f,
            Vector(0, 300), 3, Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1);

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

  if (m_on_ice && on_ground()) {
    ax *= ICE_ACCELERATION_MULTIPLIER;
  }

  m_physic.set_velocity(vx, vy);
  m_physic.set_acceleration(ax, ay);

  // we get slower when not pressing any keys
  if (dirsign == 0) {
    apply_friction();
  }

}

void
Player::do_cheer()
{
  do_duck();
  do_backflip();
  do_standup(false);
}

void
Player::do_duck() {
  if (m_duck)
    return;
  if (!is_big())
    return;

  if (!m_swimming && !m_water_jump && m_physic.get_velocity_y() != 0)
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
Player::do_standup(bool force_standup) {
  if (!m_duck)
    return;
  if (!is_big())
    return;
  if (m_backflipping)
    return;
  if (m_stone)
    return;

  if (m_swimming ? adjust_height(TUX_WIDTH) : adjust_height(BIG_TUX_HEIGHT)) {
    m_duck = false;
    m_unduck_hurt_timer.stop();
  } else if (force_standup) {
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

  m_backflip_direction = (m_dir == Direction::LEFT)?(+1):(-1);
  m_backflipping = true;
  do_jump((m_player_status.bonus == AIR_BONUS) ? -720.0f : -580.0f);
  SoundManager::current()->play("sounds/flip.wav");
  m_backflip_timer.start(TUX_BACKFLIP_TIME);
}

void
Player::do_jump(float yspeed) {
  if (!m_can_walljump && !m_in_walljump_tile && !on_ground() && !m_coyote_timer.started())
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
  if (m_controller->pressed(Control::JUMP)) m_jump_button_timer.start(JUMP_GRACE_TIME);
  if (m_controller->hold(Control::JUMP) && m_jump_button_timer.started() && (m_can_jump || m_coyote_timer.started())) {
    m_jump_button_timer.stop();
    if (m_duck) {
      // when running, only jump a little bit; else do a backflip
      if ((m_physic.get_velocity_x() != 0) ||
          (m_controller->hold(Control::LEFT)) ||
          (m_controller->hold(Control::RIGHT)))
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
      if (m_player_status.bonus == AIR_BONUS)
        do_jump((fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) ? -620.0f : -580.0f);
      else
        do_jump((fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) ? -580.0f : -520.0f);
    }
    //Stop the coyote timer only after calling do_jump, because do_jump also checks for the timer
    m_coyote_timer.stop();
    // airflower glide only when holding jump key
  } else  if (m_controller->hold(Control::JUMP) && m_player_status.bonus == AIR_BONUS && m_physic.get_velocity_y() > MAX_GLIDE_YM) {
      if (m_ability_time > 0 && !m_ability_timer.started())
        m_ability_timer.start(m_ability_time);
      else if (m_ability_timer.started()) {
        // glide stops after some duration or if buttjump is initiated
        if ((m_ability_timer.get_timeleft() <= 0.05f) || m_controller->hold(Control::DOWN)) {
          m_ability_time = 0;
          m_ability_timer.stop();
        } else {
          m_physic.set_velocity_y(MAX_GLIDE_YM);
          m_physic.set_acceleration_y(0);
        }
      }
    }


  // Let go of jump key
  else if (!m_controller->hold(Control::JUMP)) {
    if (!m_backflipping && m_jumping && m_physic.get_velocity_y() < 0) {
      m_jumping = false;
      early_jump_apex();
    }
    if (m_player_status.bonus == AIR_BONUS && m_ability_timer.started()){
      m_ability_time = m_ability_timer.get_timeleft();
      m_ability_timer.stop();
    }
  }

  if (m_jump_early_apex && m_physic.get_velocity_y() >= 0) {
    do_jump_apex();
  }

  /* In case the player has pressed Down while in a certain range of air,
     enable butt jump action */
  if (m_controller->hold(Control::DOWN) && !m_duck && is_big() && !on_ground()) {
    m_wants_buttjump = true;
    if (m_physic.get_velocity_y() >= BUTTJUMP_MIN_VELOCITY_Y) m_does_buttjump = true;
  }

  /* When Down is not held anymore, disable butt jump */
  if (!m_controller->hold(Control::DOWN)) {
    m_wants_buttjump = false;
    m_does_buttjump = false;
  }

  //The real walljumping magic
  if (m_controller->pressed(Control::JUMP) && m_can_walljump && !m_backflipping)
  {
    SoundManager::current()->play((is_big()) ? "sounds/bigjump.wav" : "sounds/jump.wav");
    m_physic.set_velocity_x(m_player_status.bonus == AIR_BONUS ?
      m_on_left_wall ? 480.f : -480.f : m_on_left_wall ? 380.f : -380.f);
    do_jump(-520.f);
  }

 m_physic.set_acceleration_y(0);
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
  if (m_swimming) {
    handle_input_swimming();
  }
  else
  {
    if (m_water_jump)
    {
      swim(0,0,0);
    }
  }

  if (!m_swimming)
  {
    if (!m_water_jump && !m_backflipping) m_sprite->set_angle(0);
    if (!m_jump_early_apex) {
      m_physic.set_gravity_modifier(1.0f);
    }
    else {
      m_physic.set_gravity_modifier(JUMP_EARLY_APEX_FACTOR);
    }
  }

  /* Peeking */
  if ( m_controller->released( Control::PEEK_LEFT ) || m_controller->released( Control::PEEK_RIGHT ) ) {
    m_peekingX = Direction::AUTO;
  }
  if ( m_controller->released( Control::PEEK_UP ) || m_controller->released( Control::PEEK_DOWN ) ) {
    m_peekingY = Direction::AUTO;
  }
  if ( m_controller->pressed( Control::PEEK_LEFT ) ) {
    m_peekingX = Direction::LEFT;
  }
  if ( m_controller->pressed( Control::PEEK_RIGHT ) ) {
    m_peekingX = Direction::RIGHT;
  }
  if (!m_backflipping && !m_jumping && on_ground()) {
    if ( m_controller->pressed( Control::PEEK_UP ) ) {
      m_peekingY = Direction::UP;
    } else if ( m_controller->pressed( Control::PEEK_DOWN ) ) {
      m_peekingY = Direction::DOWN;
    }
  }

  /* Handle horizontal movement: */
  if (!m_backflipping && !m_stone && !m_swimming) handle_horizontal_input();

  /* Jump/jumping? */
  if (on_ground())
    m_can_jump = true;

  /* Handle vertical movement: */
  if (!m_stone && !m_swimming) handle_vertical_input();

  /* Shoot! */
  auto active_bullets = Sector::get().get_object_count<Bullet>();
  if (m_controller->pressed(Control::ACTION) && (m_player_status.bonus == FIRE_BONUS || m_player_status.bonus == ICE_BONUS)) {
    if ((m_player_status.bonus == FIRE_BONUS &&
      active_bullets < m_player_status.max_fire_bullets) ||
      (m_player_status.bonus == ICE_BONUS &&
      active_bullets < m_player_status.max_ice_bullets))
    {
      Vector pos = get_pos() + Vector(m_col.m_bbox.get_width() / 2.f, m_col.m_bbox.get_height() / 2.f);
      Direction swim_dir;
      swim_dir = ((std::abs(m_swimming_angle) <= math::PI_2)
        || (m_water_jump && std::abs(m_physic.get_velocity_x()) < 10.f)) ? Direction::RIGHT : Direction::LEFT;
      if (m_swimming || m_water_jump)
      {
        m_dir = swim_dir;
      }
      Sector::get().add<Bullet>(pos, (m_swimming || m_water_jump) ?
        m_physic.get_velocity() + (Vector(std::cos(m_swimming_angle), std::sin(m_swimming_angle)) * 600.f) :
        Vector(((m_dir == Direction::RIGHT ? 600.f : -600.f) + m_physic.get_velocity_x()), 0.f),
        m_dir, m_player_status.bonus);
      SoundManager::current()->play("sounds/shoot.wav");
      m_shooting_timer.start(SHOOTING_TIME);
    }
  }

  /* Turn to Stone */
  if (m_controller->pressed(Control::DOWN) && m_player_status.bonus == EARTH_BONUS && !m_cooldown_timer.started() && on_ground() && !m_swimming) {
    if (m_controller->hold(Control::ACTION) && !m_ability_timer.started()) {
      m_ability_timer.start(static_cast<float>(m_player_status.max_earth_time) * STONE_TIME_PER_FLOWER);
      m_powersprite->stop_animation();
      m_stone = true;
      m_physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
    }
  }

  if (m_stone)
    apply_friction();

  /* Revert from Stone */
  if (m_stone && (!m_controller->hold(Control::ACTION) || m_ability_timer.get_timeleft() <= 0.5f)) {
    m_cooldown_timer.start(m_ability_timer.get_timegone()/2.0f); //The longer stone form is used, the longer until it can be used again
    m_ability_timer.stop();
    m_sprite->set_angle(0.0f);
    m_powersprite->set_angle(0.0f);
    m_lightsprite->set_angle(0.0f);
    m_stone = false;
    for (int i = 0; i < 8; i++)
    {
      Vector ppos = Vector(m_col.m_bbox.get_left() + 8.0f + 16.0f * static_cast<float>(static_cast<int>(i / 4)),
                           m_col.m_bbox.get_top() + 16.0f * static_cast<float>(i % 4));
      float grey = graphicsRandom.randf(.4f, .8f);
      Color pcolor = Color(grey, grey, grey);
      Sector::get().add<Particles>(ppos, -60, 240, 42.0f, 81.0f, Vector(0.0f, 500.0f),
                                                                8, pcolor, 4 + graphicsRandom.randf(-0.4f, 0.4f),
                                                                0.8f + graphicsRandom.randf(0.0f, 0.4f), LAYER_OBJECTS + 2);
    }
  }

  /* Duck or Standup! */
  if (m_controller->hold(Control::DOWN) && !m_stone && !m_swimming) {
    do_duck();
  } else {
    do_standup(false);
  }

  /* grabbing */
  try_grab();

  if (!m_controller->hold(Control::ACTION) && m_grabbed_object) {
    auto moving_object = dynamic_cast<MovingObject*> (m_grabbed_object);
    if (moving_object) {
      // move the grabbed object a bit away from tux
      Rectf grabbed_bbox = moving_object->get_bbox();
      Rectf dest_;
      if (m_swimming || m_water_jump)
      {
        dest_.set_bottom(m_col.m_bbox.get_bottom() + (std::sin(m_swimming_angle) * 32.f));
        dest_.set_top(dest_.get_bottom() - grabbed_bbox.get_height());
        dest_.set_left(m_col.m_bbox.get_left() + (std::cos(m_swimming_angle) * 32.f));
        dest_.set_right(dest_.get_left() + grabbed_bbox.get_width());
      }
      else
      {
        dest_.set_bottom(m_col.m_bbox.get_top() + m_col.m_bbox.get_height() * 0.66666f);
        dest_.set_top(dest_.get_bottom() - grabbed_bbox.get_height());

        if (m_dir == Direction::LEFT)
        {
          dest_.set_right(m_col.m_bbox.get_left() - 1);
          dest_.set_left(dest_.get_right() - grabbed_bbox.get_width());
        }
        else
        {
          dest_.set_left(m_col.m_bbox.get_right() + 1);
          dest_.set_right(dest_.get_left() + grabbed_bbox.get_width());
        }
      }

      if (Sector::get().is_free_of_tiles(dest_, true) &&
         Sector::get().is_free_of_statics(dest_, moving_object, true))
      {
        moving_object->set_pos(dest_.p1());
        if (m_controller->hold(Control::UP))
        {
          m_grabbed_object->ungrab(*this, Direction::UP);
        }
        else if (m_controller->hold(Control::DOWN))
        {
          m_grabbed_object->ungrab(*this, Direction::DOWN);
        }
        else if (m_swimming || m_water_jump)
        {
          m_grabbed_object->ungrab(*this,
            std::abs(m_swimming_angle) <= math::PI_2 ? Direction::RIGHT : Direction::LEFT);
        }
        else
        {
          m_grabbed_object->ungrab(*this, m_dir);
        }
        moving_object->del_remove_listener(m_grabbed_object_remove_listener.get());
        m_grabbed_object = nullptr;
      }
    } else {
      log_debug << "Non MovingObject grabbed?!?" << std::endl;
    }
  }

  /* stop backflipping at will */
  if ( m_backflipping && ( !m_controller->hold(Control::JUMP) && !m_backflip_timer.started()) ){
    stop_backflipping();
  }
}

void
Player::position_grabbed_object()
{
  auto moving_object = dynamic_cast<MovingObject*>(m_grabbed_object);
  assert(moving_object);
  const auto& object_bbox = moving_object->get_bbox();
  if (!m_swimming && !m_water_jump)
  {
    // Position where we will hold the lower-inner corner
    Vector pos(m_col.m_bbox.get_left() + m_col.m_bbox.get_width() / 2,
      m_col.m_bbox.get_top() + m_col.m_bbox.get_height()*0.66666f);
    // Adjust to find the grabbed object's upper-left corner
    if (m_dir == Direction::LEFT)
      pos.x -= object_bbox.get_width();
    pos.y -= object_bbox.get_height();
    m_grabbed_object->grab(*this, pos, m_dir);
  }
  else
  {
    Vector pos(m_col.m_bbox.get_left() + (std::cos(m_swimming_angle) * 32.f),
               m_col.m_bbox.get_top() + (std::sin(m_swimming_angle) * 32.f));
    m_grabbed_object->grab(*this, pos, m_dir);
  }
}

void
Player::try_grab()
{
  if (m_controller->hold(Control::ACTION) && !m_grabbed_object && !m_duck)
  {

    Vector pos(0.0f, 0.0f);
    if (!m_swimming && !m_water_jump)
    {
      if (m_dir == Direction::LEFT)
      {
        pos = Vector(m_col.m_bbox.get_left() - 5, m_col.m_bbox.get_bottom() - 16);
      }
      else
      {
        pos = Vector(m_col.m_bbox.get_right() + 5, m_col.m_bbox.get_bottom() - 16);
      }
    }
    else
    {
      pos = Vector(m_col.m_bbox.get_left() + 16.f + (std::cos(m_swimming_angle) * 48.f),
                   m_col.m_bbox.get_top() + 16.f + (std::sin(m_swimming_angle) * 48.f));
    }

    for (auto& moving_object : Sector::get().get_objects_by_type<MovingObject>())
    {
      Portable* portable = dynamic_cast<Portable*>(&moving_object);
      if (portable && portable->is_portable())
      {
        // make sure the Portable isn't currently non-solid
        if (moving_object.get_group() == COLGROUP_DISABLED) continue;

        // check if we are within reach
        if (moving_object.get_bbox().contains(pos))
        {
          if (m_climbing)
            stop_climbing(*m_climbing);
          m_grabbed_object = portable;

          moving_object.add_remove_listener(m_grabbed_object_remove_listener.get());

          position_grabbed_object();
          break;
        }
      }
    }
  }
}

void
Player::handle_input_ghost()
{
  float vx = 0;
  float vy = 0;
  if (m_controller->hold(Control::LEFT)) {
    m_dir = Direction::LEFT;
    vx -= MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Control::RIGHT)) {
    m_dir = Direction::RIGHT;
    vx += MAX_RUN_XM * 2;
  }
  if ((m_controller->hold(Control::UP)) || (m_controller->hold(Control::JUMP))) {
    vy -= MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Control::DOWN)) {
    vy += MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Control::ACTION)) {
    set_ghost_mode(false);
  }
  m_physic.set_velocity(vx, vy);
  m_physic.set_acceleration(0, 0);
}

void
Player::add_coins(int count)
{
  m_player_status.add_coins(count);
}

int
Player::get_coins() const
{
  return m_player_status.coins;
}

BonusType
Player::string_to_bonus(const std::string& bonus) const {
  BonusType type = NO_BONUS;

  if (bonus == "grow") {
    type = GROWUP_BONUS;
  } else if (bonus == "fireflower") {
    type = FIRE_BONUS;
  } else if (bonus == "iceflower") {
    type = ICE_BONUS;
  } else if (bonus == "airflower") {
    type = AIR_BONUS;
  } else if (bonus == "earthflower") {
    type = EARTH_BONUS;
  } else if (bonus == "none") {
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
    if (m_player_status.bonus != NO_BONUS)
      return true;
  }

  return set_bonus(type, animate);
}

bool
Player::set_bonus(BonusType type, bool animate)
{
  if (m_dying) {
    return false;
  }

  if ((m_player_status.bonus == NO_BONUS) && (type != NO_BONUS)) {
    if (!m_swimming)
    {
      if (!adjust_height(BIG_TUX_HEIGHT))
      {
        log_debug << "Can't adjust Tux height" << std::endl;
        return false;
      }
    }
    if (animate) {
      m_growing = true;
      if (m_climbing)
        m_sprite->set_action((m_dir == Direction::LEFT) ? "grow-ladder-left" : "grow-ladder-right", 1);   
      else
        m_sprite->set_action((m_dir == Direction::LEFT) ? "grow-left" : "grow-right", 1);
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
    Vector ppos = Vector((m_col.m_bbox.get_left() + m_col.m_bbox.get_right()) / 2, m_col.m_bbox.get_top());
    Vector pspeed = Vector(((m_dir == Direction::LEFT) ? 100.0f : -100.0f), -300.0f);
    Vector paccel = Vector(0, 1000);
    std::string action = (m_dir == Direction::LEFT) ? "left" : "right";
    std::string particle_name = "";

    if ((m_player_status.bonus == FIRE_BONUS) && (animate)) {
      // visually lose helmet
      if (g_config->christmas_mode) {
        particle_name = "santatux-hat";
      }
      else {
        particle_name = "firetux-helmet";
      }
    }
    if ((m_player_status.bonus == ICE_BONUS) && (animate)) {
      // visually lose cap
      particle_name = "icetux-cap";
    }
    if ((m_player_status.bonus == AIR_BONUS) && (animate)) {
      // visually lose hat
      particle_name = "airtux-hat";
    }
    if ((m_player_status.bonus == EARTH_BONUS) && (animate)) {
      // visually lose hard-hat
      particle_name = "earthtux-hardhat";
    }
    if (!particle_name.empty() && animate) {
      Sector::get().add<SpriteParticle>("images/particles/" + particle_name + ".sprite",
                                             action, ppos, ANCHOR_TOP, pspeed, paccel, LAYER_OBJECTS - 1);
    }

    m_player_status.max_fire_bullets = 0;
    m_player_status.max_ice_bullets = 0;
    m_player_status.max_air_time = 0;
    m_player_status.max_earth_time = 0;
  }
  if (type == FIRE_BONUS) m_player_status.max_fire_bullets++;
  if (type == ICE_BONUS) m_player_status.max_ice_bullets++;
  if (type == AIR_BONUS) m_player_status.max_air_time++;
  if (type == EARTH_BONUS) m_player_status.max_earth_time++;

  if (!m_second_growup_sound_timer.started() &&
     type > GROWUP_BONUS && type != m_player_status.bonus)
  {
    m_second_growup_sound_timer.start(0.5);
  }

  m_player_status.bonus = type;
  return true;
}

void
Player::set_visible(bool visible_)
{
  m_visible = visible_;
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

  if (!m_visible)
    return;

  // if Tux is above camera, draw little "air arrow" to show where he is x-wise
  if (m_col.m_bbox.get_bottom() - 16 < Sector::get().get_camera().get_translation().y) {
    float px = m_col.m_bbox.get_left() + (m_col.m_bbox.get_right() - m_col.m_bbox.get_left() - static_cast<float>(m_airarrow.get()->get_width())) / 2.0f;
    float py = Sector::get().get_camera().get_translation().y;
    py += std::min(((py - (m_col.m_bbox.get_bottom() + 16)) / 4), 16.0f);
    context.color().draw_surface(m_airarrow, Vector(px, py), LAYER_HUD - 1);
  }

  std::string sa_prefix = "";
  std::string sa_postfix = "";

  if (m_player_status.bonus == GROWUP_BONUS)
    sa_prefix = "big";
  else if (m_player_status.bonus == FIRE_BONUS)
    if (g_config->christmas_mode)
      sa_prefix = "santa";
    else
      sa_prefix = "fire";
  else if (m_player_status.bonus == ICE_BONUS)
    sa_prefix = "ice";
  else if (m_player_status.bonus == AIR_BONUS)
    sa_prefix = "air";
  else if (m_player_status.bonus == EARTH_BONUS)
    sa_prefix = "earth";
  else
    sa_prefix = "small";
  if (!m_swimming && !m_water_jump)
  {
    sa_postfix = (m_dir == Direction::RIGHT) ? "-right" : "-left";
  }
  else
  {
    sa_postfix = ((std::abs(m_swimming_angle) <= math::PI_2)
      || (m_water_jump && std::abs(m_physic.get_velocity_x()) < 10.f))
      ? "-right" : "-left";
  }

  /* Set Tux sprite action */
  if (m_dying) {
    m_sprite->set_action("gameover");
  }
  else if (m_growing)
  {
    m_sprite->set_action_continued(m_swimming || m_water_jump ?
      "swimgrow"+sa_postfix : "grow"+sa_postfix);
    // while growing, do not change action
    // do_duck() will take care of cancelling growing manually
    // update() will take care of cancelling when growing completed
  }
  else if (m_stone) {
    m_sprite->set_action(m_sprite->get_action()+"-stone");
  }
  else if (m_climbing) {
    m_sprite->set_action(sa_prefix+"-climbing"+sa_postfix);

    // Avoid flickering briefly after growing on ladder
    if ((m_physic.get_velocity_x()==0)&&(m_physic.get_velocity_y()==0))
      m_sprite->stop_animation();
  }
  else if (m_backflipping) {
    m_sprite->set_action(sa_prefix+"-backflip"+sa_postfix);
  }
  else if (m_duck && is_big() && !m_swimming) {
    m_sprite->set_action(sa_prefix+"-duck"+sa_postfix);
  }
  else if (m_skidding_timer.started() && !m_skidding_timer.check() && !m_swimming) {
    m_sprite->set_action(sa_prefix+"-skid"+sa_postfix);
  }
  else if (m_kick_timer.started() && !m_kick_timer.check() && !m_swimming && !m_water_jump) {
    m_sprite->set_action(sa_prefix+"-kick"+sa_postfix);
  }
  else if ((m_wants_buttjump || m_does_buttjump) && is_big() && !m_water_jump) {
    m_sprite->set_action(sa_prefix+"-buttjump"+sa_postfix, 1);
  }
  else if ((m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT)) && m_can_walljump)
  {
    m_sprite->set_action(sa_prefix+"-walljump"+(m_on_left_wall ? "-left" : "-right"), 1);
  }
  else if (!on_ground() || m_fall_mode != ON_GROUND)
  {
    if (m_physic.get_velocity_x() != 0 || m_fall_mode != ON_GROUND)
    {
      if (m_swimming || m_water_jump)
      {
        if (m_water_jump && m_dir != m_old_dir)
          log_debug << "Obracanko (:" << std::endl;
        if (glm::length(m_physic.get_velocity()) < 50.f)
          m_sprite->set_action(sa_prefix + "-floating" + sa_postfix);
        else if (m_water_jump)
          m_sprite->set_action(sa_prefix + "-swimjump" + sa_postfix);
        else
          m_sprite->set_action(sa_prefix + "-swimming" + sa_postfix);
      }
      else
      {
        if (m_physic.get_velocity_y() > 0)
          m_sprite->set_action(sa_prefix + "-fall" + sa_postfix);
        else if (m_physic.get_velocity_y() <= 0)
          m_sprite->set_action(sa_prefix + "-jump" + sa_postfix);
      }
    }
  }
  else
  {
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
      if (fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM && !is_big()) {
        m_sprite->set_action(sa_prefix+"-run"+sa_postfix);
      } else {
        m_sprite->set_action(sa_prefix+"-walk"+sa_postfix);
      }
    }
  }

  /* Set Tux powerup sprite action */
  if (m_player_status.has_hat_sprite())
  {
    m_powersprite->set_action(m_sprite->get_action());
    if (m_powersprite->get_frames() == m_sprite->get_frames())
    {
      m_powersprite->set_frame(m_sprite->get_current_frame());
      m_powersprite->set_frame_progress(m_sprite->get_current_frame_progress());
    }
    if (m_player_status.bonus == EARTH_BONUS)
    {
      m_lightsprite->set_action(m_sprite->get_action());
      if (m_lightsprite->get_frames() == m_sprite->get_frames())
      {
        m_lightsprite->set_frame(m_sprite->get_current_frame());
        m_lightsprite->set_frame_progress(m_sprite->get_current_frame_progress());
      }
    }
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
  if (m_safe_timer.started() && size_t(g_game_time*40)%2)
    ;  // don't draw Tux
  else if (m_player_status.bonus == EARTH_BONUS){ // draw special effects with earthflower bonus
    // shake at end of maximum stone duration
    Vector shake_delta = (m_stone && m_ability_timer.get_timeleft() < 1.0f) ? Vector(graphicsRandom.randf(-3.0f, 3.0f) * 1.0f, 0) : Vector(0,0);
    m_sprite->draw(context.color(), get_pos() + shake_delta, LAYER_OBJECTS + 1);
    // draw hardhat
    m_powersprite->draw(context.color(), get_pos() + shake_delta, LAYER_OBJECTS + 1);
    // light
    m_lightsprite->draw(context.light(), get_pos(), 0);

    // give an indicator that stone form cannot be used for a while
    if (m_cooldown_timer.started() && graphicsRandom.rand(0, 4) == 0) {
      float px = graphicsRandom.randf(m_col.m_bbox.get_left(), m_col.m_bbox.get_right());
      float py = m_col.m_bbox.get_bottom()+8;
      Vector ppos = Vector(px, py);
      Sector::get().add<SpriteParticle>(
        "images/particles/sparkle.sprite", "dark",
        ppos, ANCHOR_MIDDLE, Vector(0, 0), Vector(0, 0), LAYER_OBJECTS+1+5);
    }
  }
  else {
    if (m_dying)
      m_sprite->draw(context.color(), get_pos(), Sector::get().get_foremost_layer());
    else
      m_sprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);

    if (m_player_status.has_hat_sprite())
      m_powersprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);
  }

}

void
Player::collision_tile(uint32_t tile_attributes)
{
  if (tile_attributes & Tile::HURTS)
  {
    Rectf hurtbox = get_bbox().grown(-6.f);
    if (!Sector::get().is_free_of_tiles(hurtbox, true, Tile::HURTS))
      kill(false);
  }

  if (tile_attributes & Tile::WALLJUMP)
  {
    m_in_walljump_tile = true;
  }

  if (tile_attributes & Tile::ICE) {
    m_ice_this_frame = true;
    m_on_ice = true;
  }
}

void
Player::collision_solid(const CollisionHit& hit)
{
  if (hit.bottom) {
    if (m_physic.get_velocity_y() > 0)
      m_physic.set_velocity_y(0);

    if (!m_swimming)
      m_on_ground_flag = true;
    m_floor_normal = hit.slope_normal;

    // Butt Jump landed
    if (m_does_buttjump) {
      m_does_buttjump = false;
      m_physic.set_velocity_y(-300);
      m_on_ground_flag = false;
      Sector::get().add<Particles>(
        m_col.m_bbox.p2(),
        50, 70, 260, 280, Vector(0, 300), 3,
        Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1);
      Sector::get().add<Particles>(
        Vector(m_col.m_bbox.get_left(), m_col.m_bbox.get_bottom()),
        -70, -50, 260, 280, Vector(0, 300), 3,
        Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS+1);
      Sector::get().get_camera().shake(.1f, 0, 5);
    }

  } else if (hit.top) {
    if (m_physic.get_velocity_y() < 0)
      m_physic.set_velocity_y(.2f);
  }

  if ((hit.left || hit.right) && hit.slope_normal.x == 0) {
    m_physic.set_velocity_x(0);
  }

  // crushed?
  if (hit.crush) {
    if (hit.left || hit.right) {
      kill(true);
    } else if (hit.top || hit.bottom) {
      kill(false);
    }
  }

  if ((hit.left && m_boost < 0.f) || (hit.right && m_boost > 0.f))
    m_boost = 0.f;
}

HitResponse
Player::collision(GameObject& other, const CollisionHit& hit)
{
  auto bullet = dynamic_cast<Bullet*> (&other);
  if (bullet) {
    return FORCE_MOVE;
  }

  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    return ABORT_MOVE;
  }

  if (hit.left || hit.right) {
    try_grab(); //grab objects right now, in update it will be too late
  }
  assert(dynamic_cast<MovingObject*> (&other) != nullptr);
  auto moving_object = static_cast<MovingObject*> (&other);
  if (moving_object->get_group() == COLGROUP_TOUCHABLE) {
    auto trigger = dynamic_cast<TriggerBase*> (&other);
    if (trigger && !m_deactivated) {
      if (m_controller->pressed(Control::UP))
        trigger->event(*this, TriggerBase::EVENT_ACTIVATE);
    }

    return FORCE_MOVE;
  }

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if (badguy != nullptr) {
    if (m_safe_timer.started() || m_invincible_timer.started())
      return FORCE_MOVE;
    if (m_stone)
      return ABORT_MOVE;
  }

  return CONTINUE;
}

void
Player::make_invincible()
{
  SoundManager::current()->play("sounds/invincible_start.ogg");
  m_invincible_timer.start(TUX_INVINCIBLE_TIME);
  Sector::get().get_singleton_by_type<MusicObject>().play_music(HERRING_MUSIC);
}

void
Player::kill(bool completely)
{
  if (m_dying || m_deactivated || is_winning() )
    return;

  if (!completely && (m_safe_timer.started() || m_invincible_timer.started() || m_stone))
    return;

  m_growing = false;

  if (m_climbing) stop_climbing(*m_climbing);

  m_physic.set_velocity_x(0);

  m_sprite->set_angle(0.0f);
  m_powersprite->set_angle(0.0f);
  m_lightsprite->set_angle(0.0f);

  if (!completely && is_big()) {
    SoundManager::current()->play("sounds/hurt.wav");

    if (m_player_status.bonus == FIRE_BONUS
      || m_player_status.bonus == ICE_BONUS
      || m_player_status.bonus == AIR_BONUS
      || m_player_status.bonus == EARTH_BONUS) {
      m_safe_timer.start(TUX_SAFE_TIME);
      set_bonus(GROWUP_BONUS, true);
    } else if (m_player_status.bonus == GROWUP_BONUS) {
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

    if (m_player_status.can_reach_checkpoint())
    {
      for (int i = 0; i < 5; i++)
      {
        // the numbers: starting x, starting y, velocity y
        Sector::get().add<FallingCoin>(get_pos() +
                                                      Vector(graphicsRandom.randf(5.0f), graphicsRandom.randf(-32.0f, 18.0f)),
                                                      graphicsRandom.randf(-100.0f, 100.0f));
      }
      m_player_status.take_checkpoint_coins();
    }
    else
    {
      GameSession::current()->set_reset_point("", Vector(0.0f, 0.0f));
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
    Sector::get().get_effect().fade_out(3.0);
    SoundManager::current()->pause_music(3.0);
  }
}

void
Player::move(const Vector& vector)
{
  set_pos(vector);

  // Reset size to get correct hitbox if Tux was eg. ducked before moving
  if (is_big())
    m_col.set_size(TUX_WIDTH, BIG_TUX_HEIGHT);
  else
    m_col.set_size(TUX_WIDTH, SMALL_TUX_HEIGHT);
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

  if (m_col.m_bbox.get_right() > Sector::get().get_width()) {
    // Lock Tux to the size of the level, so that he doesn't fall off
    // the right side
    set_pos(Vector(Sector::get().get_width() - m_col.m_bbox.get_width(), m_col.m_bbox.get_top()));
  }

  m_falling_below_water = false;

  /* fallen out of the level? */
  if (m_swimming) {
    // If swimming, don't kill; just prevent from falling below the ground
    if ((get_pos().y > Sector::get().get_height() - 1) && (!m_ghost_mode)) {
      set_pos(Vector(get_pos().x, Sector::get().get_height() - 1));
      m_falling_below_water = true;
    }
  } else if ((get_pos().y > Sector::get().get_height()) && (!m_ghost_mode)) {
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
  if (!(m_player_status.bonus == AIR_BONUS))
    m_physic.set_velocity_y(m_controller->hold(Control::JUMP) ? -520.0f : -300.0f);
  else {
    m_physic.set_velocity_y(m_controller->hold(Control::JUMP) ? -580.0f : -340.0f);
    m_ability_time = static_cast<float>(m_player_status.max_air_time) * GLIDE_TIME_PER_FLOWER;
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
  m_dir = right ? Direction::RIGHT : Direction::LEFT;
}

void
Player::set_ghost_mode(bool enable)
{
  if (m_ghost_mode == enable)
    return;

  if (m_climbing) stop_climbing(*m_climbing);

  ungrab_object();

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
  if (m_climbing || m_swimming)
    return;

  m_climbing = &climbable;
  m_sprite->set_angle(0.0f);
  m_physic.enable_gravity(false);
  m_physic.set_velocity(0, 0);
  m_physic.set_acceleration(0, 0);
  if (m_backflipping) {
    stop_backflipping();
    do_standup(true);
  }
}

void
Player::stop_climbing(Climbable& /*climbable*/)
{
  if (!m_climbing) return;

  m_climbing = nullptr;

  ungrab_object();

  m_physic.enable_gravity(true);
  m_physic.set_velocity(0, 0);
  m_physic.set_acceleration(0, 0);

  if (m_controller->hold(Control::JUMP)) {
    m_on_ground_flag = true;
    do_jump(m_player_status.bonus == BonusType::AIR_BONUS ? -540.0f : -480.0f);
  }
  else if (m_controller->hold(Control::UP)) {
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
  if (m_controller->hold(Control::LEFT)) {
    m_dir = Direction::LEFT;
    vx -= MAX_CLIMB_XM;
  }
  if (m_controller->hold(Control::RIGHT)) {
    m_dir = Direction::RIGHT;
    vx += MAX_CLIMB_XM;
  }
  if (m_controller->hold(Control::UP) && m_col.m_bbox.get_top() > m_climbing->get_bbox().get_top()) {
    vy -= MAX_CLIMB_YM;
  }
  if (m_controller->hold(Control::DOWN)) {
    vy += MAX_CLIMB_YM;
  }
  if (m_controller->hold(Control::JUMP)) {
    if (m_can_jump) {
      stop_climbing(*m_climbing);
      return;
    }
  } else {
    m_can_jump = true;
  }
  if (m_controller->hold(Control::ACTION)) {
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
  if (object_name.empty())
  {
    return false;
  }
  if (auto object = dynamic_cast<GameObject*>(m_grabbed_object))
  {
    return object->get_name() == object_name;
  }
  return false;
}

void
Player::sideways_push(float delta)
{
  m_boost = delta;
}

void
Player::ungrab_object(GameObject* gameobject)
{
  if (!m_grabbed_object)
    return;

  // If gameobject is not null, then the function was called from the
  // ObjectRemoveListener.
  if (!gameobject)
    m_grabbed_object->ungrab(*this, m_dir);

  GameObject* go = dynamic_cast<GameObject*>(m_grabbed_object);

  if (go && m_grabbed_object_remove_listener)
    go->del_remove_listener(m_grabbed_object_remove_listener.get());

  m_grabbed_object = nullptr;
}

/* EOF */
