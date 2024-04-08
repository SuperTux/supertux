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
#include "object/brick.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/falling_coin.hpp"
#include "object/key.hpp"
#include "object/music_object.hpp"
#include "object/particles.hpp"
#include "object/portable.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/resources.hpp"
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

const int TIME_UNTIL_IDLE = 5000;
/** idle stages */
const std::vector<std::string> IDLE_STAGES
({
  "stand",
  "scratch",
  "idle"
});

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

const float MAX_SLIDE_SPEED = 700.f; /**< Max speed for sliding */
const float MAX_FALL_SLIDE_SPEED = 475.f; /**< Max slide speed that Tux can get from falling */
const float DOWN_SLIDE_ACCEL = 1000.f; /** < Acceleration for sliding DOWN slopes */
const float UP_SLIDE_ACCEL = 1100.f; /**< Acceleration for sliding UP slopes */

const float MAX_SLIDE_ROTATING_TIME = 0.15f;
const float MIN_SLIDE_ROTATING_TIME = 0.075f;

/* Tux's collision rectangle */
const float TUX_WIDTH = 31.8f;
const float RUNNING_TUX_WIDTH = 34;
const float SMALL_TUX_HEIGHT = 30.8f;
const float BIG_TUX_HEIGHT = 62.8f;
const float DUCKED_TUX_HEIGHT = 31.8f;

/* Stone Tux variables */
const float MAX_STONE_SPEED = 500.f;
const float STONE_KEY_ACCELERATION = 200.f;
const float STONE_DOWN_ACCELERATION = 300.f;
const float STONE_UP_ACCELERATION = 400.f;

/* Swim variables */
const float SWIM_SPEED = 300.f;
const float SWIM_BOOST_SPEED = 600.f;
const float SWIM_TO_BOOST_ACCEL = 15.f;
const float TURN_MAGNITUDE = 0.15f;
const float TURN_MAGNITUDE_BOOST = 0.2f;

/* Buttjump variables */

const float BUTTJUMP_WAIT_TIME = 0.2f; // the length of time that the buttjump action is being played
const float BUTTJUMP_SPEED = 800.f;

} // namespace

Player::Player(PlayerStatus& player_status, const std::string& name_, int player_id) :
  ExposedObject<Player, scripting::Player>(this),
  m_id(player_id),
  m_target(nullptr),
  m_deactivated(false),
  m_controller(&InputManager::current()->get_controller(player_id)),
  m_scripting_controller(new CodeController()),
  m_player_status(player_status),
  m_duck(false),
  m_crawl(false),
  m_dead(false),
  m_dying(false),
  m_winning(false),
  m_backflipping(false),
  m_backflip_direction(0),
  m_peekingX(Direction::AUTO),
  m_peekingY(Direction::AUTO),
  m_stone(false),
  m_sliding(false),
  m_slidejumping(false),
  m_swimming(false),
  m_swimboosting(false),
  m_no_water(true),
  m_on_left_wall(false),
  m_on_right_wall(false),
  m_in_walljump_tile(false),
  m_can_walljump(false),
  m_boost(0.f),
  m_speedlimit(0), //no special limit
  m_velocity_override(),
  m_scripting_controller_old(nullptr),
  m_jump_early_apex(false),
  m_on_ice(false),
  m_ice_this_frame(false),
  //m_santahatsprite(SpriteManager::current()->create("images/creatures/tux/santahat.sprite")),
  m_multiplayer_arrow(SpriteManager::current()->create("images/engine/hud/arrowdown.png")),
  m_tag_timer(),
  m_tag_fade(nullptr),
  m_tag_alpha(1.f),
  m_has_moved(false),
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
  m_buttjump_stomp(false),
  m_does_buttjump(false),
  m_invincible_timer(),
  m_skidding_timer(),
  m_safe_timer(),
  m_kick_timer(),
  m_buttjump_timer(),
  m_dying_timer(),
  m_second_growup_sound_timer(),
  m_growing(false),
  m_backflip_timer(),
  m_physic(),
  m_visible(true),
  m_grabbed_object(nullptr),
  m_grabbed_object_remove_listener(new GrabListener(*this)),
  m_released_object(false),
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
  m_unduck_hurt_timer(),
  m_idle_timer(),
  m_idle_stage(0),
  m_climbing(nullptr),
  m_ending_direction(0),
  m_collected_keys(),
  m_last_sliding_angle(0.0f),
  m_current_sliding_angle(0.0f),
  m_target_sliding_angle(0.0f),
  m_sliding_rotation_timer(),
  m_is_slidejump_falling(false),
  m_was_crawling_before_slide(false)
{
  m_name = name_;
  m_idle_timer.start(static_cast<float>(TIME_UNTIL_IDLE) / 1000.0f);

  SoundManager::current()->preload("sounds/bigjump.wav");
  SoundManager::current()->preload("sounds/brick.wav");
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
  //m_santahatsprite->set_angle(0.0f);

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
  m_speedlimit = newlimit;
}

void
Player::set_id(int id)
{
  m_id = id;
  m_controller = &(InputManager::current()->get_controller(id));
}

void
Player::set_controller(const Controller* controller_)
{
  m_controller = controller_;
}

void
Player::set_winning()
{
  if (!is_winning()) {
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

void
Player::move_to_sector(Sector& other)
{
  stop_climbing(*m_climbing);
  if (m_grabbed_object)
  {
    auto grabbed_game_object = dynamic_cast<GameObject*>(m_grabbed_object);
    if (grabbed_game_object)
      get_parent()->move_object(grabbed_game_object->get_uid(), other);
  }

  for (Key* key : m_collected_keys)
    get_parent()->move_object(key->get_uid(), other);

  // Move the player.
  get_parent()->move_object(get_uid(), other);
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

  GameSession::current()->start_sequence(this, seq, data);
}

void
Player::update(float dt_sec)
{
  if (is_dead() || Sector::get().get_object_count<Player>() == 1)
  {
    m_tag_timer.stop();
    m_tag_fade = nullptr;
    m_tag_alpha = 0.f;
    m_has_moved = true;
  }

  if (m_tag_timer.check())
  {
    m_tag_timer.stop();
    m_tag_fade = std::make_unique<FadeHelper>(1.f, 0.f, 1.f);
  }

  if (m_tag_fade)
  {
    m_tag_alpha = m_tag_fade->update(dt_sec);
    if (m_tag_fade->completed())
    {
      m_tag_fade = nullptr;
    }
  }

  // Skip if in multiplayer respawn
  if (is_dead() && m_target && Sector::get().get_object_count<Player>([this](const Player& p) { return !p.is_dead() && !p.is_dying() && !p.is_winning() && &p != this; }))
  {
    auto* target = Sector::get().get_object_by_uid<Player>(*m_target);
    if (!target || target->is_dying() || target->is_dead() || target->is_winning())
    {
      next_target();
    }

    // Respawn input is handled outside handle_input() because it happens while the player is dead
    if (is_dead() && m_target)
    {
      if (m_controller->pressed(Control::ACTION))
      {
        multiplayer_respawn();
      }
      else if (m_controller->pressed(Control::LEFT))
      {
        prev_target();
      }
      else if (m_controller->pressed(Control::RIGHT))
      {
        next_target();
      }
    }

    return;
  }

  check_bounds();

  //catch-all for other circumstances in which Tux's hitbox can't be properly adjusted
  if (is_big() &&
    !m_duck && !m_swimming && !m_water_jump && !m_backflipping && !m_sliding && !m_stone &&
    !adjust_height(BIG_TUX_HEIGHT))
  {
    //Force Tux's box up a little in order to not phase into floor
    adjust_height(BIG_TUX_HEIGHT, 10.f);
  }

  if (m_velocity_override && glm::length(m_physic.get_velocity()) < SWIM_BOOST_SPEED) {
    m_velocity_override = false;
  }

  //handling of swimming

#ifdef SWIMMING
  if (!m_ghost_mode)
  {
    if (m_no_water)
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
      if (is_big() && !m_stone && !adjust_height(BIG_TUX_HEIGHT))
      {
        //Force Tux's box up a little in order to not phase into floor
        adjust_height(BIG_TUX_HEIGHT, 10.f);
        do_duck();
      }
      else if (!is_big() || m_stone)
      {
        adjust_height(SMALL_TUX_HEIGHT);
      }
      m_dir = (m_physic.get_velocity_x() >= 0.f) ? Direction::RIGHT : Direction::LEFT;
      m_water_jump = false;
      m_swimboosting = false;
      //m_santahatsprite->set_angle(0.f);
    }
    m_no_water = true;

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
        m_no_water = false;
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
      if (can_swim_here && !m_climbing)
      {
        if (m_stone)
          stop_rolling();
        m_sliding = false;
        m_slidejumping = false;
        m_was_crawling_before_slide = false;
        m_no_water = false;
        m_water_jump = false;
        m_swimming = true;
        m_swimming_angle = math::angle(Vector(m_physic.get_velocity_x(), m_physic.get_velocity_y()));
        if (is_big())
          adjust_height(TUX_WIDTH);
        m_wants_buttjump = m_does_buttjump = m_backflipping = false;
        m_dir = (m_physic.get_velocity_x() > 0) ? Direction::LEFT : Direction::RIGHT;
        SoundManager::current()->play("sounds/splash.wav", get_pos());
      }
    }
  }
#endif

  //end of swimming handling

  if (m_dying && m_dying_timer.check()) {

    set_bonus(NO_BONUS, true);
    m_dead = true;

    if (!Sector::get().get_object_count<Player>([](const Player& p) { return !p.is_dead() && !p.is_dying(); }))
    {
      Sector::get().stop_looping_sounds();
    }
    else
    {
      next_target();
    }
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

  m_can_walljump = ((m_on_right_wall || m_on_left_wall) && !on_ground() && !m_swimming && m_in_walljump_tile && !m_stone);
  if (m_can_walljump && (m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT)) && m_physic.get_velocity_y() >= 0.f && !m_controller->pressed(Control::JUMP))
  {
    m_physic.set_velocity_y(MAX_WALLCLING_YM);
    m_physic.set_acceleration_y(0);
    if (m_water_jump)
    {
      adjust_height(is_big() ? BIG_TUX_HEIGHT : SMALL_TUX_HEIGHT);
      m_water_jump = false;
      m_swimboosting = false;
    }
    if (m_sliding)
    {
      adjust_height(is_big() ? BIG_TUX_HEIGHT : SMALL_TUX_HEIGHT);
      m_sliding = false;
      m_slidejumping = false;
    }
    //m_santahatsprite->set_angle(0.f);
  }

  m_in_walljump_tile = false;

  //End of wallclinging

  // Roll the sprite if Tux is rolling
  if (m_stone)
  {
    float f = 1.f;

    if (!std::isnan(m_floor_normal.x))
      f = std::cos(m_floor_normal.x);

    m_sprite->set_angle(m_sprite->get_angle() + m_physic.get_movement(dt_sec).x * 3.141592653898f / 2.f / f);
  }

  // extend/shrink tux collision rectangle so that we fall through/walk over 1
  // tile holes
  if (fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) {
    m_col.set_width(RUNNING_TUX_WIDTH);
  }
  else {
    m_col.set_width(TUX_WIDTH);
  }

  // on downward slopes, adjust vertical velocity so tux walks smoothly down
  if (on_ground() && !m_swimming && !m_dying) {
    if (m_floor_normal.y != 0) {
      if ((m_floor_normal.x * m_physic.get_velocity_x()) >= 0) {
        m_physic.set_velocity_y((std::abs(m_physic.get_velocity_x()) * std::abs(m_floor_normal.x)) + 100.f);
      }
    }
  }

  // handle backflipping
  if (m_backflipping && !m_dying) {
    //prevent player from changing direction when backflipping
    m_dir = (m_backflip_direction == 1) ? Direction::LEFT : Direction::RIGHT;
    if (m_backflip_timer.started()) m_physic.set_velocity_x(100.0f * static_cast<float>(m_backflip_direction));
  }

  if (on_ground()) {
    m_coyote_timer.start(COYOTE_TIME * ((m_sliding || m_stone) ? 2.f : 1.f));
  }

  // set fall mode...
  if (on_ground()) {
    m_fall_mode = ON_GROUND;
    m_last_ground_y = get_pos().y;
  }
  else {
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
      if (!m_stone && !m_sliding) {
        m_sprite->set_angle(0.0f);
        //m_santahatsprite->set_angle(0.0f);
      }

      // if controls are currently deactivated, we take care of standing up ourselves
      if (m_deactivated)
        do_standup(false);
    }
  }

  if (m_second_growup_sound_timer.check())
  {
    SoundManager::current()->play("sounds/grow.wav", get_pos());
    m_second_growup_sound_timer.stop();
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

  // when invincible, spawn particles
  if (m_invincible_timer.started())
  {
    if (graphicsRandom.rand(0, 2) == 0)
    {
      float px = graphicsRandom.randf(m_col.m_bbox.get_left() + 0, m_col.m_bbox.get_right() - 0);
      float py = graphicsRandom.randf(m_col.m_bbox.get_top() + 0, m_col.m_bbox.get_bottom() - 0);
      Vector ppos = Vector(px, py);
      Vector pspeed = Vector(0, 0);
      Vector paccel = Vector(0, 0);
      Sector::get().add<SpriteParticle>(
        "images/particles/sparkle.sprite",
        // draw bright sparkle when there is lots of time left,
        // dark sparkle when invincibility is about to end
        (m_invincible_timer.get_timeleft() > TUX_INVINCIBLE_TIME_WARNING) ?
        // make every other a longer sparkle to make trail a bit fuzzy
        (size_t(g_game_time * 20) % 2) ? "small" : "medium"
        :
        "dark", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS + 1 + 5);
    }
  }

  if (m_growing) {
    if (m_sprite->animation_done()) m_growing = false;
  }

  // when climbing animate only while moving
  if (m_climbing) {
    if ((m_physic.get_velocity_x() == 0) && (m_physic.get_velocity_y() == 0))
    {
      m_sprite->pause_animation();
      //m_santahatsprite->pause_animation();
    }
    else if (!m_growing)
    {
      m_sprite->resume_animation();
      //m_santahatsprite->resume_animation();
    }
  }

  if (m_floor_normal.y != 0.f && m_crawl)
  {
    m_crawl = false;
    m_sliding = true;
    m_was_crawling_before_slide = true;
  }

  //sliding

  if (m_sliding)
  {
    float sliding_angle = 0.0f;

    if (on_ground())
    {
      float floor_angle = 0.0f;

      if (m_floor_normal.y != 0.0f)
      {
        floor_angle = math::degrees(math::angle(m_floor_normal)) + 90.0f;
      }

      if (m_target_sliding_angle != floor_angle)
      {
        const float current_velocity = glm::length(m_physic.get_velocity());
        constexpr float max_velocity = 500.0f;
        const float rotation_time = MIN_SLIDE_ROTATING_TIME + (1.0f - (std::min(max_velocity, current_velocity) / max_velocity)) * (MAX_SLIDE_ROTATING_TIME - MIN_SLIDE_ROTATING_TIME);
        m_last_sliding_angle = m_current_sliding_angle;
        m_target_sliding_angle = floor_angle;
        m_sliding_rotation_timer.start(rotation_time);
      }

      if (m_sliding_rotation_timer.started())
      {
        const float progress = m_sliding_rotation_timer.get_timegone() / m_sliding_rotation_timer.get_period();
        const float angle_difference = m_target_sliding_angle - m_last_sliding_angle;

        sliding_angle = m_current_sliding_angle = m_last_sliding_angle + (angle_difference * progress);
      }
      else
      {
        sliding_angle = m_last_sliding_angle = floor_angle;
      }
    }
    else
    {
      if (!m_jumping && !m_is_slidejump_falling)
      {
        sliding_angle = math::degrees(math::angle(m_physic.get_velocity()));
        if (m_physic.get_velocity_x() < 0.0f)
        {
          sliding_angle -= 180.0f;
        }
        m_target_sliding_angle = m_current_sliding_angle = sliding_angle;
      }
      else
      {
        // Do not rotate while slidejump animation is displayed
        sliding_angle = 0.0f;
      }
    }
    m_sprite->set_angle(sliding_angle);

    //if you stop holding down when sliding, then it stops.
    //or, stop sliding if you come to a stop and are not on a slope.
    if (!m_controller->hold(Control::DOWN) ||
      (m_floor_normal.y == 0.f && std::abs(m_physic.get_velocity_x()) <= 1.f))
    {
      if (is_big())
      {
        if (m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT)) {
          m_crawl = true;
        }
        m_duck = true;
      }
      m_sliding = false;
      m_slidejumping = false;
      m_was_crawling_before_slide = false;
    }
  }

  if (m_sliding || m_stone)
  {
    Rectf sidebrickbox = get_bbox().grown(-1.f);
    sidebrickbox.set_left(get_bbox().get_left() + (m_dir == Direction::LEFT ? -12.f : 1.f));
    sidebrickbox.set_right(get_bbox().get_right() + (m_dir == Direction::RIGHT ? 12.f : -1.f));

    for (auto& brick : Sector::get().get_objects_by_type<Brick>()) {
      if (sidebrickbox.overlaps(brick.get_bbox()) && (m_stone || (m_sliding && brick.get_class_name() != "heavy-brick")) &&
        std::abs(m_physic.get_velocity_x()) >= 150.f) {
        brick.try_break(this, is_big());
      }
    }
  }

  if (m_does_buttjump || (m_stone && m_physic.get_velocity_y() > 30.f && !m_coyote_timer.started()))
  {
    Rectf downbox = get_bbox().grown(-1.f);
    downbox.set_top(get_bbox().get_bottom());
    downbox.set_bottom(downbox.get_bottom() + 16.f);
    for (auto& brick : Sector::get().get_objects_by_type<Brick>()) {
      // stoneform breaks through any kind of bricks
      if (downbox.overlaps(brick.get_bbox()) && (m_stone || !dynamic_cast<HeavyBrick*>(&brick)))
        brick.try_break(this, is_big());
    }
    for (auto& badguy : Sector::get().get_objects_by_type<BadGuy>()) {
      if (downbox.overlaps(badguy.get_bbox()) && badguy.is_snipable() && !badguy.is_grabbed())
        badguy.kill_fall();
    }
  }

  // break bricks above without stopping
  if (m_stone && m_physic.get_velocity_y() < 30.f)
  {
    Rectf topbox = get_bbox().grown(-1.f);
    topbox.set_top(get_bbox().get_top() - 16.f);
    for (auto& brick : Sector::get().get_objects_by_type<Brick>()) {
      if (topbox.overlaps(brick.get_bbox()))
        brick.try_break(this, is_big());
    }
  }

  //launch from slopes

  Rectf launchbox = get_bbox();
  launchbox.set_bottom(get_bbox().get_bottom() - 8.f);
  launchbox.set_left(get_bbox().get_left() + (m_dir == Direction::LEFT ? -32.f : 33.f));
  launchbox.set_right(get_bbox().get_right() + (m_dir == Direction::RIGHT ? 32.f : -33.f));
  if (m_sliding && on_ground() && m_floor_normal.y != 0 && m_floor_normal.x * m_physic.get_velocity_x() < 0.f &&
    Sector::get().is_free_of_statics(launchbox) && !m_slidejumping)
  {
    m_slidejumping = true;
    m_physic.set_velocity_y(-glm::length(m_physic.get_velocity()) * std::abs(m_floor_normal.x));
  }
  else if (m_sliding && on_ground()) {
    m_slidejumping = false;
  }

  m_ice_this_frame = false;
  m_on_ground_flag = false;
}

void
Player::slide()
{
  if (m_swimming || m_water_jump || m_stone)
  {
    m_sliding = false;
    m_was_crawling_before_slide = false;
    return;
  }
  m_sliding = true;

  if (m_physic.get_velocity_x() > 0.f) {
    m_dir = Direction::RIGHT;
  }
  else if (m_physic.get_velocity_x() < 0.f) {
    m_dir = Direction::LEFT;
  }

  //pre_slide helps us detect the ground where Tux is about to slide on because sometimes on_ground() doesn't work or isn't relevant
  Rectf pre_slide_box = get_bbox();
  float fast_fall_speed = m_physic.get_velocity_y() <= 400.f ? 0.f : m_physic.get_velocity_y()*0.03f;
  pre_slide_box.set_bottom(m_col.m_bbox.get_bottom() + fast_fall_speed + 16.f);
  bool pre_slide = !Sector::get().is_free_of_statics(pre_slide_box);

  if (std::abs(m_physic.get_velocity_x()) > MAX_SLIDE_SPEED) {
    m_physic.set_acceleration_x(-m_physic.get_velocity_x());
  }
  else
  {
    if (!pre_slide) {
      m_physic.set_acceleration_x(0.f);
    }
    else
    {
      //handle adding acceleration from falling down
      if (m_sliding && !on_ground() && m_floor_normal.x*m_physic.get_velocity_x() <= 0.f && m_physic.get_velocity_y() > 0.f)
      {
        //less max momentum when getting it on flat surfaces
        if (std::abs(m_physic.get_velocity_x()) + (std::abs(m_physic.get_velocity_y())*0.2f) <= MAX_FALL_SLIDE_SPEED) {
          m_physic.set_velocity_x(m_physic.get_velocity_x() + (std::abs(m_physic.get_velocity_y())*(m_dir == Direction::LEFT ? -0.125f : 0.125f)));
        }
        else {
          m_physic.set_velocity_x(MAX_FALL_SLIDE_SPEED * (m_dir == Direction::LEFT ? -1.f : 1.f));
        }
      }

      //handle adding acceleration from sliding down, removing it from reaching floor or an incline
      if (m_floor_normal.y == 0.f && m_can_jump)
      {
        if (!m_slidejumping && !m_jumping) {
          apply_friction();
        }
      }
      else
      {
        if (m_floor_normal.x > 0.f) {
          m_physic.set_acceleration_x((m_dir == Direction::LEFT ? UP_SLIDE_ACCEL : DOWN_SLIDE_ACCEL)*std::abs(m_floor_normal.x));
        }
        if (m_floor_normal.x < 0.f) {
          m_physic.set_acceleration_x((m_dir == Direction::RIGHT ? -UP_SLIDE_ACCEL : -DOWN_SLIDE_ACCEL)*std::abs(m_floor_normal.x));
        }
      }
    }
  }
}

void
Player::handle_input_swimming()
{
  float pointx = float(m_controller->hold(Control::RIGHT)) - float(m_controller->hold(Control::LEFT)),
        pointy = float(m_controller->hold(Control::DOWN)) - float(m_controller->hold(Control::UP));

  bool boost = m_controller->hold(Control::JUMP);

  swim(pointx, pointy, boost);
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

      float epsilon = (boost ? TURN_MAGNITUDE : TURN_MAGNITUDE_BOOST) * delta;
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
      if (glm::length(m_physic.get_velocity()) > SWIM_SPEED)
      {
        m_physic.set_acceleration(-vx,-vy);   // Was too lazy to set it properly ~~zwatotem
      }

      // Natural friction
      if (!is_ang_defined)
      {
        m_physic.set_acceleration(-3.f*vx, -3.f*vy);
      }

      //not boosting? let's slow this penguin down!!!
      if (!boost && is_ang_defined && glm::length(m_physic.get_velocity()) > (SWIM_SPEED + 10.f))
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
        if (glm::length(m_physic.get_velocity()) < SWIM_BOOST_SPEED)
        {
          m_swimboosting = true;
          if (is_ang_defined)
          {
            vx += SWIM_TO_BOOST_ACCEL * pointx;
            vy += SWIM_TO_BOOST_ACCEL * pointy;
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
          if (glm::length(m_physic.get_velocity()) < (SWIM_SPEED + 10.f))
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
    //m_santahatsprite->set_angle(math::degrees(m_swimming_angle));
  }
  else
  {
    // otherwise angle the sprite normally
    float angle = (std::abs(m_swimming_angle) <= math::PI_2) ?
                    math::degrees(m_swimming_angle) :
                    math::degrees(math::PI + m_swimming_angle);

    m_sprite->set_angle(angle);
    //m_santahatsprite->set_angle(angle);

    //Force the speed to point in the direction Tux is going unless Tux is being pushed by something else
    if (m_swimming && !m_water_jump && boost && m_boost == 0.f && !m_velocity_override)
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

void
Player::apply_friction()
{
  bool is_on_ground = on_ground();
  float velx = m_physic.get_velocity_x();
  if (is_on_ground && (fabsf(velx) < (m_stone ? 5.f : WALK_SPEED))) {
    m_physic.set_velocity_x(0);
    m_physic.set_acceleration_x(0);
    return;
  }
  float friction = WALK_ACCELERATION_X;
  if (m_on_ice && is_on_ground)
    //we need this or else sliding on ice will cause Tux to go on for a very long time
    friction *= (ICE_FRICTION_MULTIPLIER*(m_sliding ? 4.f : m_stone ? 5.f : 1.f));
  else
    friction *= (NORMAL_FRICTION_MULTIPLIER*(m_sliding ? 0.8f : m_stone ? 0.4f : 1.f));
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
  float ax = 0;
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

  if (m_duck && (m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT))) {
    m_crawl = true;
  }

  if (m_crawl && on_ground() && std::abs(m_physic.get_velocity_x()) < WALK_SPEED)
  {
    if (m_controller->hold(Control::LEFT) && !m_controller->hold(Control::RIGHT))
    {
      vx = -WALK_SPEED;
      m_dir = Direction::LEFT;
    }
    else if (m_controller->hold(Control::RIGHT) && !m_controller->hold(Control::LEFT))
    {
      vx = WALK_SPEED;
      m_dir = Direction::RIGHT;
    }
    else {
      vx = 0.f;
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
    if (vx >= MAX_RUN_XM + BONUS_RUN_XM *((get_bonus() == AIR_BONUS) ? 1 : 0)) {
      ax = std::min(ax, -OVERSPEED_DECELERATION);
    } else if (vx <= -MAX_RUN_XM - BONUS_RUN_XM * ((get_bonus() == AIR_BONUS) ? 1 : 0)) {
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
        SoundManager::current()->play("sounds/skid.wav", get_pos());
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
  if (m_sliding && Sector::get().is_free_of_statics(Rectf(get_bbox().get_left(), get_bbox().get_top() - 32.f,
    get_bbox().get_right(), get_bbox().get_bottom())))
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
  if (!m_duck || !is_big() || m_backflipping || m_stone)
  {
    m_crawl = false;
    return;
  }

  Rectf new_bbox = m_col.m_bbox;
  float new_height = m_swimming ? TUX_WIDTH : BIG_TUX_HEIGHT;
  new_bbox.move(Vector(0, m_col.m_bbox.get_height() - new_height));
  new_bbox.set_height(new_height);
  if (!Sector::get().is_free_of_movingstatics(new_bbox, this) && !force_standup)
  {
    m_crawl = true;
    return;
  }

  if (m_swimming ? adjust_height(TUX_WIDTH) : adjust_height(BIG_TUX_HEIGHT)) {
    m_duck = false;
    m_crawl = false;
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
  if (m_crawl)
    return;
  if (!on_ground())
    return;

  m_backflip_direction = (m_dir == Direction::LEFT)?(+1):(-1);
  m_backflipping = true;
  do_jump((get_bonus() == AIR_BONUS) ? -720.0f : -580.0f);
  SoundManager::current()->play("sounds/flip.wav", get_pos());
  m_backflip_timer.start(TUX_BACKFLIP_TIME);
}

void
Player::do_jump(float yspeed) {
  if (!m_can_walljump && !m_in_walljump_tile && !on_ground() && !m_coyote_timer.started())
    return;

  // jump only if it would make Tux go faster upwards
  if (m_physic.get_velocity_y() > yspeed) {
    m_physic.set_velocity_y(yspeed * (m_sliding ? 0.75f : 1.f));
    //bbox.move(Vector(0, -1));
    m_jumping = true;
    m_on_ground_flag = false;
    m_can_jump = false;

    /*if (m_sliding)
    {
      if (!adjust_height(is_big() ? BIG_TUX_HEIGHT : SMALL_TUX_HEIGHT)) {
        m_duck = true;
      }
      m_sliding = false;
      m_slidejumping = false;
    }*/

    // play sound
    if (is_big()) {
      SoundManager::current()->play("sounds/bigjump.wav", get_pos());
    } else {
      SoundManager::current()->play("sounds/jump.wav", get_pos());
    }
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
      if (get_bonus() == AIR_BONUS)
        do_jump((fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) ? -620.0f : -580.0f);
      else
        do_jump((fabsf(m_physic.get_velocity_x()) > MAX_WALK_XM) ? -580.0f : -520.0f);
    }
    //Stop the coyote timer only after calling do_jump, because do_jump also checks for the timer
    m_coyote_timer.stop();
    // airflower glide only when holding jump key
  }
  else if (m_controller->hold(Control::JUMP) && get_bonus() == AIR_BONUS && m_physic.get_velocity_y() > MAX_GLIDE_YM) {
    // glide stops if buttjump is initiated
    if (!m_controller->hold(Control::DOWN))
    {
      m_physic.set_velocity_y(MAX_GLIDE_YM);
      m_physic.set_acceleration_y(0);
    }
  }


  // Let go of jump key
  else if (!m_controller->hold(Control::JUMP)) {
    if (!m_backflipping && m_jumping && m_physic.get_velocity_y() < 0) {
      m_jumping = false;

      if (m_sliding)
      {
        m_is_slidejump_falling = true;
      }

      early_jump_apex();
    }
  }

  if (m_jump_early_apex && m_physic.get_velocity_y() >= 0) {
    do_jump_apex();
  }

  /* In case the player has pressed Down while in a certain range of air,
     enable butt jump action */
  if (m_controller->hold(Control::DOWN) && !m_duck && is_big() && !on_ground() && !m_sliding && !m_stone) {
    if (!m_wants_buttjump && !m_does_buttjump) {
      m_buttjump_timer.start(BUTTJUMP_WAIT_TIME);
    }
    m_wants_buttjump = true;
    if (m_buttjump_timer.check())
    {
      m_buttjump_timer.stop();
      m_does_buttjump = true;
    }
    if (m_does_buttjump) {
      m_physic.set_velocity_y(BUTTJUMP_SPEED);
    }
  }

  /* When Down is not held anymore, disable butt jump */
  if (!m_controller->hold(Control::DOWN)) {
    m_wants_buttjump = false;
    m_does_buttjump = false;
    m_buttjump_stomp = false;
  }

  //The real walljumping magic
  if (m_controller->pressed(Control::JUMP) && m_can_walljump && !m_backflipping)
  {
    SoundManager::current()->play((is_big()) ? "sounds/bigjump.wav" : "sounds/jump.wav", get_pos());
    m_physic.set_velocity_x(get_bonus() == AIR_BONUS ?
      m_on_left_wall ? 480.f : -480.f : m_on_left_wall ? 380.f : -380.f);
    do_jump(-520.f);
  }

 m_physic.set_acceleration_y(0);
}

void
Player::handle_input()
{
  // Display the player's ID on top of them at the beginning of the level/sector
  // and persist the number until the player moves, because players will be
  // stacked upon spawning.
  // It is probably possible to displace the player without touching left or
  // right, but for simplicity, only those can make the player number vanish.
  if (!m_has_moved && (m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT)))
  {
    m_has_moved = true;
    m_tag_timer.start(1.f);
  }

  if (m_ghost_mode) {
    handle_input_ghost();
    return;
  }
  if (m_climbing) {
    handle_input_climbing();
    return;
  }
  if (m_stone) {
    handle_input_rolling();
    return;
  }
  if (m_swimming) {
    handle_input_swimming();
  }
  else
  {
    if (m_water_jump)
    {
      swim(0, 0, 0);
    }
  }

  if (!m_swimming)
  {
    if (!m_water_jump && !m_backflipping && !m_sliding)
    {
      m_sprite->set_angle(0);
      //m_santahatsprite->set_angle(0);
    }

    if (!m_jump_early_apex) {
      m_physic.set_gravity_modifier(1.0f);
    }
    else {
      m_physic.set_gravity_modifier(JUMP_EARLY_APEX_FACTOR);
    }
  }

  /* Peeking */
  if (!m_controller->hold( Control::PEEK_LEFT ) && !m_controller->hold( Control::PEEK_RIGHT))
    m_peekingX = Direction::AUTO;
  if (!m_controller->hold( Control::PEEK_UP ) && !m_controller->hold( Control::PEEK_DOWN))
    m_peekingY = Direction::AUTO;

  if (m_controller->pressed(Control::PEEK_LEFT))
    m_peekingX = Direction::LEFT;
  else if (m_controller->pressed(Control::PEEK_RIGHT))
    m_peekingX = Direction::RIGHT;

  if (m_controller->pressed(Control::PEEK_UP))
    m_peekingY = Direction::UP;
  else if (m_controller->pressed(Control::PEEK_DOWN))
    m_peekingY = Direction::DOWN;

  /* Handle horizontal movement: */
  if (!m_backflipping && !m_stone && !m_swimming && !m_sliding) handle_horizontal_input();

  /* Jump/jumping? */
  if (on_ground())
    m_can_jump = true;

  /* Handle vertical movement: */
  if (!m_stone && !m_swimming) handle_vertical_input();

  /* grabbing */
  bool just_grabbed = try_grab();

  /* Shoot! */
  auto active_bullets = Sector::get().get_object_count<Bullet>([this](const Bullet& b){ return &b.get_player() == this; });
  if (m_controller->pressed(Control::ACTION) && (get_bonus() == FIRE_BONUS || get_bonus() == ICE_BONUS) && !just_grabbed) {
    if ((get_bonus() == FIRE_BONUS &&
      active_bullets < m_player_status.max_fire_bullets[get_id()]) ||
      (get_bonus() == ICE_BONUS &&
      active_bullets < m_player_status.max_ice_bullets[get_id()]))
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
        m_dir, get_bonus(), *this);
      SoundManager::current()->play("sounds/shoot.wav", get_pos());
    }
  }

  /* Turn to Stone */
  if (m_controller->hold(Control::DOWN) && !m_does_buttjump && m_coyote_timer.started() && !m_swimming && (std::abs(m_physic.get_velocity_x()) > 150.f) && get_bonus() == EARTH_BONUS) {
    m_physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
    adjust_height(TUX_WIDTH);
    m_stone = true;
    m_swimming = false;
    m_sliding = false;
    m_was_crawling_before_slide = false;
    m_crawl = false;
    m_duck = false;
  }

  if (m_stone)
    apply_friction();

  /* Duck or Standup! */
  if ((m_controller->pressed(Control::DOWN) || ((m_duck || m_wants_buttjump) && m_controller->hold(Control::DOWN))) &&
    !m_swimming && !m_sliding && !m_stone) {
    do_duck();
  }
  else {
    do_standup(false);
  }

  /* Drop grabbed object when releasing the Action button on keyboard or gamepad, and on the second button press when using touchscreen */
  if ((m_controller->is_touchscreen() ? m_controller->pressed(Control::ACTION) : !m_controller->hold(Control::ACTION)) &&
      m_grabbed_object && !just_grabbed) {
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
        m_released_object = true;
      }
    } else {
      log_debug << "Non MovingObject grabbed?!?" << std::endl;
    }
  }

  if (!m_controller->hold(Control::ACTION) && m_released_object) {
    m_released_object = false;
  }

  /* stop backflipping at will */
  if ( m_backflipping && ( !m_controller->hold(Control::JUMP) && !m_backflip_timer.started()) ){
    stop_backflipping();
  }

  if (m_sliding)
  {
    adjust_height(DUCKED_TUX_HEIGHT);
    slide();
  }
  else if (!m_sliding && (m_coyote_timer.started()) && !m_skidding_timer.started() &&
    (m_floor_normal.y != 0 || (m_controller->hold(Control::LEFT) || m_controller->hold(Control::RIGHT)))
    && m_controller->pressed(Control::DOWN) && std::abs(m_physic.get_velocity_x()) > 1.f &&
    get_bonus()!= EARTH_BONUS)
  {
    sideways_push(m_dir == Direction::LEFT ? -100.f : 100.f);
    adjust_height(DUCKED_TUX_HEIGHT);
    slide();
  }
}

void
Player::position_grabbed_object(bool teleport)
{
  if (!m_grabbed_object)
    return;

  auto moving_object = dynamic_cast<MovingObject*>(m_grabbed_object);
  assert(moving_object);
  const auto& object_bbox = moving_object->get_bbox();

  Vector pos;
  if (!m_swimming && !m_water_jump)
  {
    // Position where we will hold the lower-inner corner
    pos = Vector(m_col.m_bbox.get_left() + m_col.m_bbox.get_width() / 2,
                 m_col.m_bbox.get_top() + m_col.m_bbox.get_height() * 0.66666f);
    // Adjust to find the grabbed object's upper-left corner
    if (m_dir == Direction::LEFT)
      pos.x -= object_bbox.get_width();
    pos.y -= object_bbox.get_height();
  }
  else
  {
    pos = Vector(m_col.m_bbox.get_left() + (std::cos(m_swimming_angle) * 32.f),
                 m_col.m_bbox.get_top() + (std::sin(m_swimming_angle) * 32.f));
  }

  if (teleport)
    moving_object->set_pos(pos);
  m_grabbed_object->grab(*this, pos, m_dir);
}

bool
Player::try_grab()
{
  if (m_controller->hold(Control::ACTION) && !m_grabbed_object && !m_duck && !m_released_object)
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
          return true;
        }
      }
    }
  }
  return false;
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
  if (m_controller->hold(Control::UP)) {
    vy -= MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Control::DOWN)) {
    vy += MAX_RUN_XM * 2;
  }
  if (m_controller->hold(Control::ACTION)) {
    set_ghost_mode(false);
  }
  m_physic.set_velocity(Vector(vx, vy) * (m_controller->hold(Control::JUMP) ? 2.5f : 1.f));
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

std::string
Player::bonus_to_string() const
{
  switch(get_bonus())
  {
    case GROWUP_BONUS:
      return "grow";
    case FIRE_BONUS:
      return "fireflower";
    case ICE_BONUS:
      return "iceflower";
    case AIR_BONUS:
      return "airflower";
    case EARTH_BONUS:
      return "earthflower";
    default:
      return "none";
  }
}

bool
Player::add_bonus(const std::string& bonustype)
{
  return add_bonus(string_to_bonus(bonustype));
}

bool
Player::set_bonus(const std::string& bonustype)
{
  return set_bonus(string_to_bonus(bonustype));
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
    if (get_bonus() != NO_BONUS)
      return true;
  }

  return set_bonus(type, animate);
}

bool
Player::set_bonus(BonusType type, bool animate, bool increment_powerup_counter)
{
  if (m_dying) {
    return false;
  }

  if ((get_bonus() == NO_BONUS) && (type != NO_BONUS || m_stone)) {
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
        m_sprite->set_action("climbgrow", m_dir, 1);
      else if (m_swimming)
        m_sprite->set_action("swimgrow", m_dir, 1);
      else if (m_sliding)
        m_sprite->set_action("slidegrow", m_dir, 1);
      else
        m_sprite->set_action("grow", m_dir , 1);
    }
  }

  if (type == NO_BONUS) {
    if (!adjust_height(SMALL_TUX_HEIGHT)) {
      log_debug << "Can't adjust Tux height" << std::endl;
      return false;
    }
    if (m_does_buttjump) m_does_buttjump = false;
  }

  if ((type == NO_BONUS) || (type == GROWUP_BONUS)) {
    m_player_status.max_fire_bullets[get_id()] = 0;
    m_player_status.max_ice_bullets[get_id()] = 0;
    m_player_status.max_air_time[get_id()] = 0;
    m_player_status.max_earth_time[get_id()] = 0;
  }

  if (increment_powerup_counter)
  {
    if (type == FIRE_BONUS) m_player_status.max_fire_bullets[get_id()]++;
    if (type == ICE_BONUS) m_player_status.max_ice_bullets[get_id()]++;
    if (type == AIR_BONUS) m_player_status.max_air_time[get_id()]++;
    if (type == EARTH_BONUS) m_player_status.max_earth_time[get_id()]++;
  }

  if (!m_second_growup_sound_timer.started() &&
     type > GROWUP_BONUS && type != get_bonus())
  {
    m_second_growup_sound_timer.start(0.5);
  }

  m_player_status.bonus[get_id()] = type;
  return true;
}

BonusType
Player::get_bonus() const
{
  return m_player_status.bonus[m_id];
}

void
Player::set_visible(bool visible)
{
  m_visible = visible;
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

std::string
Player::get_action() const
{
  return m_sprite->get_action();
}

void
Player::draw(DrawingContext& context)
{
  if(Editor::is_active())
    return;

  if (is_dead() && m_target && Sector::get().get_object_count<Player>([this](const Player& p){ return !p.is_dead() && !p.is_dying() && !p.is_winning() && &p != this; }))
  {
    auto* target = Sector::get().get_object_by_uid<Player>(*m_target);
    if (target)
    {
      Vector pos(target->get_bbox().get_middle().x, target->get_bbox().get_top() - static_cast<float>(m_multiplayer_arrow->get_height()) * 1.5f);
      Vector pos_surf(pos - Vector(static_cast<float>(m_multiplayer_arrow->get_width()) / 2.f, 0.f));
      m_multiplayer_arrow->draw(context.color(), pos_surf, LAYER_LIGHTMAP + 1);
      context.color().draw_text(Resources::normal_font, std::to_string(get_id() + 1), pos,
                                FontAlignment::ALIGN_CENTER, LAYER_LIGHTMAP + 1);
    }
    return;
  }

  if (m_tag_alpha > 0.f)
  {
    context.color().draw_text(Resources::normal_font, std::to_string(get_id() + 1),
                              m_col.m_bbox.get_middle() - Vector(0.f, Resources::normal_font->get_height() / 2.f),
                              FontAlignment::ALIGN_CENTER, LAYER_LIGHTMAP + 1,
                              Color(1.f, 1.f, 1.f, m_tag_alpha));
  }

  // if Tux is above camera, draw little "air arrow" to show where he is x-wise
  if (m_col.m_bbox.get_bottom() - 16 < Sector::get().get_camera().get_translation().y) {
    float px = m_col.m_bbox.get_left() + (m_col.m_bbox.get_right() - m_col.m_bbox.get_left() - static_cast<float>(m_airarrow.get()->get_width())) / 2.0f;
    float py = Sector::get().get_camera().get_translation().y;
    py += std::min(((py - (m_col.m_bbox.get_bottom() + 16)) / 4), 16.0f);
    context.color().draw_surface(m_airarrow, Vector(px, py), LAYER_HUD - 1);
  }

  std::string sa_prefix = "";
  std::string sa_postfix = "";

  if (get_bonus() == GROWUP_BONUS)
    sa_prefix = "big";
  else if (get_bonus() == FIRE_BONUS)
    sa_prefix = "fire";
  else if (get_bonus() == ICE_BONUS)
    sa_prefix = "ice";
  else if (get_bonus() == AIR_BONUS)
    sa_prefix = "air";
  else if (get_bonus() == EARTH_BONUS)
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
    m_sprite->set_angle(0.0f);
    m_sprite->set_action("gameover");
  }
  else if (m_growing)
  {
    // while growing, do not change action
    // do_duck() will take care of cancelling growing manually
    // update() will take care of cancelling when growing completed
    std::string action = "grow";
    if (m_swimming || m_water_jump) {
      action = "swimgrow";
    }
    else if (m_sliding) {
      action = "slidegrow";
    }
    else if (m_climbing) {
      action = "climbgrow";
    }
    m_sprite->set_action(action + sa_postfix, Sprite::LOOPS_CONTINUED);
  }
  else if (m_stone) {
    m_sprite->set_action("earth-stone");
  }
  else if (m_climbing) {
    m_sprite->set_action(sa_prefix+"-climb"+sa_postfix);

    // Avoid flickering briefly after growing on ladder
    if ((m_physic.get_velocity_x()==0)&&(m_physic.get_velocity_y()==0))
      m_sprite->pause_animation();
  }
  else if (m_backflipping) {
    m_sprite->set_action(sa_prefix+"-backflip"+sa_postfix);
  }
  else if (m_sliding) {
    if (m_jumping || m_is_slidejump_falling) {
      m_sprite->set_action(sa_prefix +"-slidejump"+ sa_postfix);
    }
    else {
      m_sprite->set_action(sa_prefix + "-slide" + sa_postfix);
      if (m_was_crawling_before_slide)
      {
        m_sprite->set_frame(m_sprite->get_frames()); // Skip the "duck" animation when coming from crawling
        m_was_crawling_before_slide = false;
      }
    }
  }
  else if (m_duck && is_big() && !m_swimming && !m_crawl) {
    m_sprite->set_action(sa_prefix+"-duck"+sa_postfix);
  }
  else if (m_crawl)
  {
    if (on_ground())
    {
      m_sprite->set_action(sa_prefix + "-crawl" + sa_postfix);
      if (m_physic.get_velocity_x() != 0.f) {
        m_sprite->resume_animation();
      }
      else {
        m_sprite->pause_animation();
      }
    }
    else {
      m_sprite->set_action(sa_prefix + "-slidejump" + sa_postfix);
    }
  }
  else if (m_skidding_timer.started() && !m_skidding_timer.check() && !m_swimming) {
    m_sprite->set_action(sa_prefix + "-skid" + sa_postfix);
  }
  else if (m_kick_timer.started() && !m_kick_timer.check() && !m_swimming && !m_water_jump) {
    m_sprite->set_action(sa_prefix+"-kick"+sa_postfix);
  }
  else if ((m_wants_buttjump || m_does_buttjump) && is_big() && !m_water_jump) {
    if (m_buttjump_stomp) {
      m_sprite->set_action(sa_prefix + "-stomp" + sa_postfix, 1);
    }
    else {
      m_sprite->set_action(sa_prefix + "-buttjump" + sa_postfix, 1);
    }
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
          m_sprite->set_action(sa_prefix + "-float" + sa_postfix);
        else if (m_water_jump)
          m_sprite->set_action(sa_prefix + "-swimjump" + sa_postfix);
        else if (m_swimboosting)
          m_sprite->set_action(sa_prefix + "-boost" + sa_postfix);
        else
          m_sprite->set_action(sa_prefix + "-swim" + sa_postfix);
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
      if (std::all_of(IDLE_STAGES.begin(), IDLE_STAGES.end(),
            [this](const std::string& stage) { return m_sprite->get_action().find("-" + stage + "-") == std::string::npos; }))
      {
        m_idle_stage = 0;
        m_idle_timer.start(static_cast<float>(TIME_UNTIL_IDLE) / 1000.0f);

        m_sprite->set_action(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix, Sprite::LOOPS_CONTINUED);
      }
      else if (m_idle_timer.check() || m_sprite->animation_done()) {
        m_idle_stage++;
        if (m_idle_stage >= static_cast<unsigned int>(IDLE_STAGES.size()))
        {
          m_idle_stage = static_cast<int>(IDLE_STAGES.size()) - 1;
          m_sprite->set_action(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix);
          m_sprite->set_animation_loops(-1);
        }
        else
        {
          m_sprite->set_action(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix, 1);
        }
      }
      else {
        m_sprite->set_action(sa_prefix+("-" + IDLE_STAGES[m_idle_stage])+sa_postfix, Sprite::LOOPS_CONTINUED);
      }
    }
    else
    {
      if (std::abs(m_physic.get_velocity_x()) >= MAX_RUN_XM-3)
      {
        m_sprite->set_action(sa_prefix+"-run"+sa_postfix);
      }
      else
      {
        m_sprite->set_action(sa_prefix+"-walk"+sa_postfix);
      }
    }
  }

  /* Set Tux powerup sprite action */
  if (g_config->christmas_mode)
  {
    //TODO: Implement new santa hats
    //m_santahatsprite->set_action(m_sprite->get_action());
    /*if (m_santahatsprite->get_frames() == m_sprite->get_frames())
    {
      m_santahatsprite->set_frame(m_sprite->get_current_frame());
      m_santahatsprite->set_frame_progress(m_sprite->get_current_frame_progress());
    }*/
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
  if (!m_visible || (m_safe_timer.started() && size_t(g_game_time * 40) % 2))
  {
  }  // don't draw Tux

  else if (m_dying)
    m_sprite->draw(context.color(), get_pos(), Sector::get().get_foremost_opaque_layer() + 1);
  else
    m_sprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);

  //TODO: Replace recoloring with proper costumes
  Color power_color = (get_bonus() == FIRE_BONUS ? Color(1.f, 0.7f, 0.5f) :
    get_bonus() == ICE_BONUS ? Color(0.7f, 1.f, 1.f) :
    get_bonus() == AIR_BONUS ? Color(0.7f, 1.f, 0.5f) :
    get_bonus() == EARTH_BONUS ? Color(1.f, 0.9f, 0.6f) :
    Color(1.f, 1.f, 1.f));

  m_sprite->set_color(m_stone ? Color(1.f, 1.f, 1.f) : power_color);
}


void
Player::collision_tile(uint32_t tile_attributes)
{
  if (tile_attributes & Tile::HURTS)
  {
    if (tile_attributes & Tile::UNISOLID)
      kill(false);
    else
    {
      Rectf hurtbox = get_bbox().grown(-6.f);
      if (!Sector::get().is_free_of_tiles(hurtbox, false, Tile::HURTS))
        kill(false);
    }
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
    m_is_slidejump_falling = false;
    m_slidejumping = false;

    // Butt Jump landed
    if (m_does_buttjump) {
      m_does_buttjump = false;
      m_buttjump_stomp = true;
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
      Sector::get().get_camera().shake(.1f, 0.f, 10.f);
    }

  } else if (hit.top) {
    if (m_physic.get_velocity_y() < 0)
      m_physic.set_velocity_y(.2f);
  }

  if (m_stone && m_floor_normal.y == 0 && (((m_physic.get_velocity_x() < -MAX_RUN_XM) && hit.left) ||
    ((m_physic.get_velocity_x() > MAX_RUN_XM) && hit.right)))
  {
    m_physic.set_acceleration_x(0);
    m_physic.set_velocity_x(0);
    stop_rolling();
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
Player::on_flip(float height)
{
  Vector pos = get_pos();
  pos.y = height - pos.y - get_bbox().get_height();
  move(pos);
}

void
Player::remove_me()
{
  InputManager::current()->on_player_removed(get_id());
  MovingObject::remove_me();
}

void
Player::make_invincible()
{
  // No get_pos() here since the music affects the whole sector
  SoundManager::current()->play("sounds/invincible_start.ogg");
  m_invincible_timer.start(TUX_INVINCIBLE_TIME);
  Sector::get().get_singleton_by_type<MusicObject>().play_music(HERRING_MUSIC);
}

void
Player::kill(bool completely)
{
  if (m_dying || m_deactivated || is_winning() )
    return;

  if (!completely && (m_safe_timer.started() || m_invincible_timer.started()))
    return;

  m_growing = false;

  if (m_climbing) stop_climbing(*m_climbing);

  m_physic.set_velocity_x(0);
  m_boost = 0.f;

  m_sprite->set_angle(0.0f);
  //m_santahatsprite->set_angle(0.0f);

  if (!completely && is_big()) {
    SoundManager::current()->play("sounds/hurt.wav", get_pos());

    if (get_bonus() == FIRE_BONUS
      || get_bonus() == ICE_BONUS
      || get_bonus() == AIR_BONUS
      || get_bonus() == EARTH_BONUS) {
      m_safe_timer.start(TUX_SAFE_TIME);
      set_bonus(GROWUP_BONUS, true);
    } else if (get_bonus() == GROWUP_BONUS) {
      m_safe_timer.start(TUX_SAFE_TIME /* + GROWING_TIME */);
      m_duck = false;
      stop_backflipping();
      set_bonus(NO_BONUS, true);
    }
  } else {
    SoundManager::current()->play("sounds/kill.wav", get_pos());

    auto* session = GameSession::current();
    if (session && session->m_prevent_death &&
                   !session->reset_checkpoint_button)
    {
      set_ghost_mode(true);
      return;
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

    auto alive_players = Sector::get().get_object_count<Player>([](const Player& p){ return !p.is_dead() && !p.is_dying(); });

    if (!alive_players)
    {
      if (m_player_status.respawns_at_checkpoint())
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

      Sector::get().get_effect().fade_out(3.0);
      SoundManager::current()->pause_music(3.0);
    }
  }

  //Sector::get().get_camera().shake(0.1f, m_dying ? 32.f : 0.f, m_dying ? 20.f : 10.f);
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

  // Make sure objects following Tux move directly with him
  position_grabbed_object(true);
  for (Key* key : m_collected_keys)
    key->update_pos();

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
    set_pos(Vector(Sector::get().get_width() - m_col.m_bbox.get_width(),
                   m_col.m_bbox.get_top()));
  }

  // If Tux is swimming, don't allow him to go below the sector
  if (m_swimming && !m_ghost_mode && !is_dying() && !is_dead()
      && m_col.m_bbox.get_bottom() > Sector::get().get_height()) {
    set_pos(Vector(m_col.m_bbox.get_left(),
                   Sector::get().get_height() - m_col.m_bbox.get_height()));
  }

  /* fallen out of the level? */
  if ((get_pos().y > Sector::get().get_height()) && (!m_ghost_mode)) {
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
  if (!(get_bonus() == AIR_BONUS))
    m_physic.set_velocity_y(m_controller->hold(Control::JUMP) ? -520.0f : -300.0f);
  else {
    m_physic.set_velocity_y(m_controller->hold(Control::JUMP) ? -580.0f : -340.0f);
  }
}

//scripting Functions Below

void
Player::deactivate()
{
  if (m_deactivated)
    return;
  m_deactivated = true;
  m_physic.set_velocity(0, 0);
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
Player::start_climbing(Climbable& climbable)
{
  if (m_climbing || m_swimming || m_stone)
    return;

  m_climbing = &climbable;
  m_sprite->set_angle(0.0f);
  m_boost = 0.f;
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

  if (m_controller->hold(Control::JUMP) && !m_controller->hold(Control::DOWN)) {
    m_on_ground_flag = true;
    m_jump_early_apex = false;
    do_jump(get_bonus() == AIR_BONUS ? -540.0f : -480.0f);
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
  auto obj_bbox = m_climbing->get_bbox();
  if (m_controller->hold(Control::LEFT) && m_col.m_bbox.get_left() > obj_bbox.get_left()) {
    m_dir = Direction::LEFT;
    vx -= MAX_CLIMB_XM;
  }
  if (m_controller->hold(Control::RIGHT) && m_col.m_bbox.get_right() < obj_bbox.get_right()) {
    m_dir = Direction::RIGHT;
    vx += MAX_CLIMB_XM;
  }
  if (m_controller->hold(Control::UP) && m_col.m_bbox.get_top() > obj_bbox.get_top()) {
    vy -= MAX_CLIMB_YM;
  }
  if (m_controller->hold(Control::DOWN) && m_col.m_bbox.get_bottom() < obj_bbox.get_bottom()) {
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
  m_physic.set_velocity(vx, vy);
  m_physic.set_acceleration(0, 0);
}

void
Player::handle_input_rolling()
{
  // handle exiting
  if (m_stone)
  {
    if (!m_controller->hold(Control::DOWN)) {
      stop_rolling(false);
    }
    else if (get_bonus() != EARTH_BONUS) {
      stop_rolling();
    }
  }

  // handle jumping
  if (m_controller->pressed(Control::JUMP)) m_jump_button_timer.start(JUMP_GRACE_TIME);
  if (m_controller->hold(Control::JUMP) && m_jump_button_timer.started() && (m_can_jump || m_coyote_timer.started()))
  {
    m_jump_button_timer.stop();
    do_jump(-450.f);
    m_coyote_timer.stop();
  }

  // Let go of jump key
  else if (!m_controller->hold(Control::JUMP)) {
    if (!m_backflipping && m_jumping && m_physic.get_velocity_y() < 0) {
      m_jumping = false;
      early_jump_apex();
    }
  }

  if (m_jump_early_apex && m_physic.get_velocity_y() >= 0) {
    do_jump_apex();
  }

  // handle x-movement

  if (std::abs(m_physic.get_velocity_x()) > MAX_STONE_SPEED) {
    m_physic.set_acceleration_x(-m_physic.get_velocity_x());
  }
  else
  {
    // these variables are apparently used differently and must be initialized differently to avoid errors
    float ax;
    float sx = 0.f;

    // slope velocity
    if (m_floor_normal.y != 0)
    {
      if (m_floor_normal.x > 0.f) {
        sx = ((m_dir == Direction::LEFT ? STONE_UP_ACCELERATION : STONE_DOWN_ACCELERATION)*std::abs(m_floor_normal.x));
      }
      if (m_floor_normal.x < 0.f) {
        sx = ((m_dir == Direction::RIGHT ? -STONE_UP_ACCELERATION : -STONE_DOWN_ACCELERATION)*std::abs(m_floor_normal.x));
      }
    }
    else
    {
      sx = 0.f;
    }

    // key velocity
    if (m_controller->hold(Control::LEFT) && !m_controller->hold(Control::RIGHT))
    {
      ax = -STONE_KEY_ACCELERATION;
      m_dir = Direction::LEFT;
    }
    else if (m_controller->hold(Control::RIGHT) && !m_controller->hold(Control::LEFT))
    {
      ax = STONE_KEY_ACCELERATION;
      m_dir = Direction::RIGHT;
    }
    else {
      ax = 0.f;
    }

    if (m_controller->hold(Control::RIGHT) || m_controller->hold(Control::LEFT) || m_floor_normal.y != 0.f) {
      m_physic.set_acceleration_x(ax + sx);
    }
    else {
      apply_friction();
    }
  }
}

void
Player::stop_backflipping()
{
  m_backflipping = false;
  m_backflip_direction = 0;
  m_sprite->set_angle(0.0f);
  //m_santahatsprite->set_angle(0.0f);
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

void
Player::next_target()
{
  const auto& players = Sector::get().get_players();

  Player* first = nullptr;
  bool is_next = false;
  for (auto* player : players)
  {
    if (!player->is_dead() && !player->is_dying() && !player->is_winning())
    {
      if (!first)
      {
        first = player;
      }

      if (is_next)
      {
        m_target.reset(new UID());
        *m_target = player->get_uid();
        return;
      }

      if (m_target && player->get_uid() == *m_target)
      {
        is_next = true;
      }
    }
  }

  if (first)
  {
    m_target.reset(new UID());
    *m_target = first->get_uid();
  }
  else
  {
    m_target.reset(nullptr);
  }
}

void
Player::prev_target()
{
  const auto& players = Sector::get().get_players();

  Player* last = nullptr;
  for (auto* player : players)
  {
    if (!player->is_dead() && !player->is_dying() && !player->is_winning())
    {
      if (m_target && player->get_uid() == *m_target && last)
      {
        *m_target = last->get_uid();
        return;
      }

      last = player;
    }
  }

  if (last)
  {
    m_target.reset(new UID());
    *m_target = last->get_uid();
  }
  else
  {
    m_target.reset(nullptr);
  }
}

void
Player::multiplayer_prepare_spawn()
{
  m_physic.enable_gravity(true);
  m_physic.set_gravity_modifier(1.0f); // Undo jump_early_apex
  m_safe_timer.stop();
  m_invincible_timer.stop();
  m_physic.set_acceleration(0, -9999);
  m_physic.set_velocity(0, -9999);
  m_dying = true;
  set_group(COLGROUP_DISABLED);
  m_dead = true;

  next_target();
}

void
Player::multiplayer_respawn()
{
  if (!m_target)
  {
    log_warning << "Can't respawn multiplayer player, no target" << std::endl;
    return;
  }

  auto target = Sector::get().get_object_by_uid<Player>(*m_target);

  if (!target)
  {
    log_warning << "Can't respawn multiplayer player, target missing" << std::endl;
    return;
  }

  m_dying = false;
  m_dead = false;
  m_deactivated = false;
  m_ghost_mode = false;
  set_group(COLGROUP_MOVING);
  m_physic.reset();

  move(target->get_pos());
  m_target.reset();
}

void
Player::stop_rolling(bool violent)
{
  m_sprite->set_angle(0.0f);
  if (!m_swimming && !m_water_jump && !m_sliding && !m_duck)
  {
    if (!adjust_height(BIG_TUX_HEIGHT))
    {
      adjust_height(BIG_TUX_HEIGHT, 10.f);
      do_duck();
    }
  }
  if (violent)
  {
    for (int i = 0; i < 5; i++)
    {
      Vector pspeed = Vector(graphicsRandom.randf(-100.f, 100.f)*(static_cast<float>(i)-2), graphicsRandom.randf(-200.f, -150.f));
      Vector paccel = Vector(0, 1000.f + graphicsRandom.randf(-100.f, 100.f));
      Sector::get().add<SpriteParticle>(
        "images/particles/rock.sprite", "rock-"+std::to_string(i),
        get_bbox().get_middle(),
        ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS + 6, true);
    }
    SoundManager::current()->play("sounds/brick.wav", get_pos());
  }
  m_stone = false;
}

void
Player::add_collected_key(Key* key)
{
  m_collected_keys.push_back(key);
}

void
Player::remove_collected_key(Key* key)
{
  m_collected_keys.erase(std::remove(m_collected_keys.begin(),
                                     m_collected_keys.end(),
                                     key),
                         m_collected_keys.end());
}

/* EOF */
