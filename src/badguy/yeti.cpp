//  SuperTux - Boss "Yeti"
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "badguy/yeti.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "badguy/bouncing_snowball.hpp"
#include "badguy/yeti_stalactite.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/bigsnowball.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

namespace
{
const float ANNOUNCE_INTERVAL = 0.1f;

const float RUN_VX = 350; /**< Horizontal speed while running. */
const float RUN_PINCH_VX = 400; /**< Horizontal speed while running. */

const float JUMP_UP_VY = -800; /**< Vertical speed while jumping on the dais. */

const float STOMP_VY = -300; /**< Vertical speed while stomping on the dais. */

const float RUN_DISTANCE = 1060; /**< Distance between the x-coordinates of left and right end positions. */
const float JUMP_SPACE = 448; /**< Distance between the jump position and the stand position. */
const float BEFORE_WAIT = 3;
const float BALL_WAIT = 2;
const float STOMP_WAIT = 0.5; /**< Time we stay on the dais before jumping again. */
const float SAFE_TIME = 1; /**< The time we are safe when Tux just hit us. */

const float YETI_SQUISH_TIME = 3;

const float SNOW_EXPLOSIONS_FREQUENCY = 8; /**< Number of snowball explosions per second. */
const int SNOW_EXPLOSIONS_COUNT = 5; /**< Number of snowballs per explosion. */
const float SNOW_EXPLOSIONS_VX = 150; /**< Speed of snowballs. */
const float SNOW_EXPLOSIONS_VY = -200; /**< Speed of snowballs. */
}

Yeti::Yeti(const ReaderMapping& reader) :
  Boss(reader, "images/creatures/yeti/yeti.sprite"),
  m_state(ANNOUNCE),
  m_next_state(),
  m_state_timer(),
  m_safe_timer(),
  m_attacked(false),
  m_attack_count(),
  m_left_stand_x(),
  m_right_stand_x(),
  m_left_jump_x(),
  m_right_jump_x(),
  m_fixed_pos(),
  m_just_hit(),
  m_just_threw(),
  m_grabbed_tux(),
  m_jumped()
{
  reader.get("hud-icon", m_hud_icon, "images/creatures/yeti/hudlife.png");
  m_hud_head = Surface::from_file(m_hud_icon);
  SoundManager::current()->preload("sounds/yeti_gna.wav");
  SoundManager::current()->preload("sounds/yeti_roar.wav");

  //initialize();

  reader.get("fixed-pos", m_fixed_pos, false);
  if (m_fixed_pos) {
    m_left_stand_x = 216;
    m_right_stand_x = 1014;
    m_left_jump_x = 528;
    m_right_jump_x = 692;
  } else {
    recalculate_pos();
  }
}

void
Yeti::initialize()
{
  m_dir = Direction::RIGHT;
  announce();
}

void
Yeti::recalculate_pos()
{
  if (m_dir == Direction::RIGHT) {
    m_left_stand_x = m_col.m_bbox.get_left();
    m_right_stand_x = m_left_stand_x + RUN_DISTANCE;
  } else {
    m_right_stand_x = m_col.m_bbox.get_left();
    m_left_stand_x = m_right_stand_x - RUN_DISTANCE;
  }

  m_left_jump_x = m_left_stand_x + JUMP_SPACE;
  m_right_jump_x = m_right_stand_x - JUMP_SPACE;
}

void
Yeti::draw(DrawingContext& context)
{
  // We blink when we are safe.
  if (m_safe_timer.started() && size_t(g_game_time * 40) % 2)
    return;

  Boss::draw(context);
}

void
Yeti::active_update(float dt_sec)
{
  Boss::boss_update(dt_sec);
  auto player = get_nearest_player();

  float push_distance;
  push_distance = player ? (glm::length(get_bbox().get_middle() - player->get_bbox().get_middle())) : 0.f;

  if (on_ground() && (m_state == STOMP || m_state == THROW) && push_distance <= 160.f && m_physic.get_velocity_x() == 0.f)
  {
    m_state_timer.stop();
    m_physic.enable_gravity(false);
    set_group(COLGROUP_DISABLED);
    m_state = PREVENT_TUX;
  }

  switch (m_state) {
    case ANNOUNCE:
      if (m_state_timer.check())
      {
        run(false);
        jump(STOMP_VY);
      }

      if (m_sprite->animation_done())
      {
        set_action("stand", m_dir);
        m_state_timer.start(ANNOUNCE_INTERVAL);
      }

      break;

    case JUMP:
      if (m_sprite->animation_done())
        set_action("fall", m_dir);

      if (on_ground())
        run(true);

      break;

    case RUN:
      if ((m_dir == Direction::RIGHT && get_pos().x >= m_right_stand_x + get_bbox().get_size().width) ||
          (m_dir == Direction::LEFT && get_pos().x <= m_left_stand_x))
      {
        idle(true);
      }

      if ((m_dir == Direction::RIGHT && math::in_bounds(get_pos().x, m_right_jump_x, m_right_jump_x + 64.f)) ||
          (m_dir == Direction::LEFT && math::in_bounds(get_pos().x, m_left_jump_x - 64.f, m_left_jump_x)))
      {
        jump(JUMP_UP_VY);
      }

      break;

    case IDLE:
      if (m_sprite->animation_done())
      {
        m_dir = invert_dir(m_dir);
        set_action("stand", m_dir);
        m_state_timer.start(BALL_WAIT);
      }

      if (m_state_timer.check())
      {
        switch (m_next_state)
        {
          case RUN:
            run(false);
            jump(STOMP_VY);
            break;

          case THROW:
            throw_snowball();
            break;

          default:
            break;
        }
      }

      break;

    case THROW:
      if (!m_attacked && m_sprite->get_current_frame() == 5)
      {
        summon_snowball();
        m_attacked = true;
        m_attack_count++;
        break;
      }

      if (m_sprite->animation_done())
      {
        if (m_attack_count >= (m_pinch_mode ? 3 : 2))
        {
          m_next_state = RUN;
          m_attack_count = 0;
        }
        else
          m_next_state = THROW;

        m_attacked = false;
        idle(false);
      }

      break;

    default:
      break;
  }

  m_col.set_movement(m_physic.get_movement(dt_sec));
}

void
Yeti::announce()
{
  m_state = ANNOUNCE;
  set_action("rage", m_dir);
  m_state_timer.stop();

  SoundManager::current()->play("sounds/yeti_roar.wav", get_pos());
}

void
Yeti::run(bool change_state)
{
  if (change_state)
  {
    m_state = RUN;
    set_action("walk", m_dir);
    m_state_timer.start(BEFORE_WAIT);
  }

  m_physic.set_velocity_x(RUN_VX * (m_dir == Direction::RIGHT ? 1.f : -1.f));
}

void
Yeti::jump(float velocity)
{
  m_state = JUMP;
  set_action("jump", m_dir);

  m_physic.set_velocity_y(velocity);
}

void
Yeti::idle(bool stomp)
{
  m_state = IDLE;
  set_action(stomp ? "stomp" : "stand", m_dir);

  if (!stomp)
    m_state_timer.start(1);
  else
    m_next_state = THROW;

  m_physic.set_velocity_x(0);
}

void
Yeti::throw_snowball()
{
  m_state = THROW;
  set_action("throw", m_dir);

  //m_attack_count = 0;
  m_state_timer.start(STOMP_WAIT / (m_pinch_mode ? 1.2f : 1.f));
}

void
Yeti::throw_big_snowball()
{
  //m_attack_count = 0;
  m_state = THROW_BIG;
  m_state_timer.start(BALL_WAIT);
}

void
Yeti::stomp()
{
  m_attack_count = 0;
  m_state = STOMP;
  m_state_timer.start(BALL_WAIT);
}

bool
Yeti::collision_squished(MovingObject& object)
{
  if (m_state != YetiState::JUMP && m_state != YetiState::RUN && m_state != YetiState::STOMP) {
    return false;
  }

  kill_squished(object);
  return true;
}

HitResponse
Yeti::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  badguy.kill_fall();
  return FORCE_MOVE;
}

void
Yeti::kill_squished(GameObject& object)
{
  auto player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
    take_hit(*player);
  }
}

void
Yeti::take_hit(Player& )
{
  if (m_safe_timer.started() || m_just_hit ||
    std::abs(m_left_stand_x - get_pos().x) < 80 || std::abs(m_right_stand_x - get_pos().x) < 80)
    return;

  m_just_hit = true;

  SoundManager::current()->play("sounds/yeti_roar.wav", get_pos());
  m_lives--;

  if (m_lives <= 0) {
    // We're dead.
    m_physic.set_velocity((m_dir == Direction::RIGHT ? RUN_VX : -RUN_VX) / 5, 0);

    // Set the badguy layer to be above the foremost, so that
    // this does not reveal secret tilemaps:
    m_layer = Sector::get().get_foremost_opaque_layer() + 1;
    m_state = DIZZY;
    m_state_timer.start(YETI_SQUISH_TIME);
    set_colgroup_active(COLGROUP_MOVING_ONLY_STATIC);
    // sprite->setAction("dead");
  }
  else {
    m_safe_timer.start(SAFE_TIME);
  }
}

void
Yeti::kill_fall()
{
  // Shooting bullets or being invincible won't prevent this action.
}

void
Yeti::drop_stalactite()
{
  // Make a stalactite fall down and shake the camera a bit.
  Sector::get().get_camera().shake(.1f, 0, 20.f);

  auto player = get_nearest_player();
  if (!player) return;

  for (auto& stalactite : Sector::get().get_objects_by_type<YetiStalactite>())
  {
    if (stalactite.is_hanging()) {
      if (!m_pinch_mode) {
        // Drop stalactites within 3 units of player, going out with each jump.
        float distancex = fabsf(stalactite.get_bbox().get_middle().x - player->get_bbox().get_middle().x);
        if (distancex < static_cast<float>(m_attack_count) * 32.0f) {
          stalactite.start_shaking();
        }
      }
      else { /* if (hitpoints < 3) */
        // Drop every 3rd pair of stalactites.
        if ((((static_cast<int>(stalactite.get_pos().x) + 16) / 64) % 3) == (m_attack_count % 3))
          stalactite.start_shaking();
      }
    }
  }
}

void
Yeti::summon_snowball()
{
  Vector bs_pos = get_pos() + Vector(m_dir == Direction::LEFT ? -32.f : (get_bbox().get_width() + 1.f), 0.f);
  Sector::get().add<BouncingSnowball>(bs_pos, m_dir, 150.f * (m_pinch_mode ? 1.2f : 1.f));
}

void
Yeti::summon_big_snowball()
{
  Vector bs_pos = Vector(get_bbox().get_middle().x - 44.f, get_bbox().get_top() - 89.f);
  Sector::get().add<BigSnowball>(bs_pos, m_dir, true);
}

void
Yeti::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);
  if (hit.top || hit.bottom) {
    // Hit floor or roof.
    m_physic.set_velocity_y(0);
    switch (m_state) {
      case RUN:
        break;
      case JUMP:
        break;
      case THROW:
        break;
      case THROW_BIG:
        break;
      case STOMP:
        // We just landed.
        if (!m_state_timer.started())
        {
          set_action("stomp", m_dir);
          m_attack_count++;
          drop_stalactite();

          // Go to the other side after 3 jumps.
          if (m_attack_count == 3)
          {
            m_just_hit = false;
            if (m_pinch_mode) {
              throw_big_snowball();
            }
            else {
              run(true);
            }
          }
          else
          {
            // Jump again.
            m_state_timer.start(STOMP_WAIT / (m_pinch_mode ? 1.2f : 1.f));
          }
        }
        break;
      case DIZZY:
        break;
      case PREVENT_TUX:
        break;
    }
  } else if (hit.left || hit.right) {
    // Hit wall.
    if(m_state != DIZZY)
      jump(JUMP_UP_VY);
  }
}

ObjectSettings
Yeti::get_settings()
{
  ObjectSettings result = Boss::get_settings();

  result.add_bool(_("Fixed position"), &m_fixed_pos, "fixed-pos", false);

  return result;
}

void
Yeti::add_snow_explosions()
{
  // TODO: WAOW! This is terrible! Remove this!
  /*
  for (int i = 0; i < SNOW_EXPLOSIONS_COUNT; i++) {
    Vector pos = get_pos();
    Vector velocity(SNOW_EXPLOSIONS_VX * graphicsRandom.randf(0.5f, 2.0f) * (graphicsRandom.rand(2) ? 1.0f : -1.0f),
                    SNOW_EXPLOSIONS_VY * graphicsRandom.randf(0.5f, 2.0f));
    pos.x += static_cast<float>(m_sprite->get_width()) / 2.0f;
    pos.x += static_cast<float>(m_sprite->get_width()) * graphicsRandom.randf(0.3f, 0.5f) * ((velocity.x > 0) ? 1.0f : -1.0f);
    pos.y += static_cast<float>(m_sprite->get_height()) * graphicsRandom.randf(-0.3f, 0.3f);
    velocity.x += m_physic.get_velocity_x();
    Sector::get().add<SnowExplosionParticle>(pos, velocity);
  }
  */
}

Yeti::SnowExplosionParticle::SnowExplosionParticle(const Vector& pos, const Vector& velocity)
  : BadGuy(pos, (velocity.x > 0) ? Direction::RIGHT : Direction::LEFT, "images/objects/bullets/icebullet.sprite")
{
  m_physic.set_velocity(velocity);
  m_physic.enable_gravity(true);
  set_state(STATE_FALLING);
  m_layer = Sector::get().get_foremost_opaque_layer() + 1;
}

std::vector<Direction>
Yeti::get_allowed_directions() const
{
  return {};
}

/* EOF */
