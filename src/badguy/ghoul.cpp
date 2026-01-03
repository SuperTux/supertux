//  SuperTux
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

#include "badguy/ghoul.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace
{
  static constexpr float DEFAULT_TRACK_RANGE = 2500.f;
  static constexpr float RESPAWN_TIME = 4.f;

  static constexpr float DOWN_VELOCITY = 40.f;
  static constexpr float UP_VELOCITY = -150.f;
  static constexpr float UP_ACCELERATION = 150.f;
  static constexpr float HORZ_SPEED = 60.f;
  static constexpr float HORZ_ACCELERATION = 190.f;
  static constexpr float HORZ_TRUST_MULTIPLIER = 3.f;

  static constexpr float VERT_OFFSET = 48.f;
}

Ghoul::Ghoul(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/ghoul/ghoul.sprite"),
  m_track_range(),
  m_home_pos(),
  m_respawn_timer(),
  m_state()
{
  m_can_glint = false;

  reader.get("track-range", m_track_range, DEFAULT_TRACK_RANGE);

  set_action(m_dir);
  m_physic.enable_gravity(false);
  m_home_pos = get_pos();
  set_state(ROAMING_DOWN);

  SoundManager::current()->preload("sounds/ghoul_stunned.ogg");
  SoundManager::current()->preload("sounds/ghoul_recovering.ogg");
}

Vector
Ghoul::to_target() const
{
  auto player = get_nearest_player();
  if (!player)
    return Vector(0.0f, 0.0f);

  const Vector p1 = get_bbox().get_middle();
  Vector p2 = player->get_bbox().get_middle();
  p2.y -= 32.f; // a little offset, so he doesn't hit Tux from below

  const Vector dist = p2 - p1;
  return dist;
}

void
Ghoul::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  auto player = get_nearest_player();
  if (!player)
  {
    m_physic.set_acceleration(0.0f, 0.0f);
    return;
  }

  const Vector dist = to_target();
  const Direction new_dir = dist.x < 0 ? Direction::LEFT : Direction::RIGHT;
  const bool dir_changed = new_dir != m_dir;
  const bool should_chase = glm::length(dist) < m_track_range;

  switch (m_state)
  {
  case ROAMING_DOWN:
    roaming_decel_check();
    if (dir_changed)
    {
      set_action(new_dir == Direction::LEFT ? "left" : "right");
      m_dir = new_dir;
    }
    if (should_chase)
    {
      set_state(CHASING_DOWN);
    }
    else if (get_pos().y > m_home_pos.y + VERT_OFFSET)
    {
      set_state(ROAMING_ACCEL1);
    }
    break;
  case CHASING_DOWN:
    update_speed(dist);
    if (dir_changed)
    {
      set_action(new_dir == Direction::LEFT ? "left" : "right");
      m_dir = new_dir;
    }
    if (!should_chase)
    {
      m_home_pos = get_pos();
      set_state(ROAMING_DOWN);
    }
    else if (dist.y < -VERT_OFFSET)
    {
      set_state(CHASING_ACCEL1);
    }
    break;
  case ROAMING_ACCEL1:
    roaming_decel_check();
    if (m_sprite->animation_done())
    {
      set_state(ROAMING_ACCEL2);
    }
    break;
  case CHASING_ACCEL1:
    update_speed(dist);
    if (m_sprite->animation_done())
    {
      set_state(CHASING_ACCEL2);
    }
    break;
  case ROAMING_ACCEL2:
    roaming_decel_check();
    if (m_sprite->animation_done())
    {
      set_state(ROAMING_UP);
    }
    break;
  case CHASING_ACCEL2:
    update_speed(dist);
    if (m_sprite->animation_done())
    {
      set_state(CHASING_UP);
    }
    break;
  case ROAMING_UP:
    roaming_decel_check();
    if (dir_changed)
    {
      set_action(new_dir == Direction::LEFT ? "left-up" : "right-up");
      m_dir = new_dir;
    }
    if (should_chase)
    {
      set_state(CHASING_UP);
    }
    else if (m_physic.get_velocity_y() > DOWN_VELOCITY)
    {
      if (get_pos().y > m_home_pos.y + VERT_OFFSET)
      {
        set_state(ROAMING_ACCEL1);
      }
      else
      {
        set_state(ROAMING_DOWN);
      }
    }
    break;
  case CHASING_UP:
    update_speed(dist);
    if (dir_changed)
    {
      set_action(new_dir == Direction::LEFT ? "left-up" : "right-up");
      m_dir = new_dir;
    }
    if (!should_chase)
    {
      m_home_pos = get_pos();
      set_state(ROAMING_UP);
    }
    else if (m_physic.get_velocity_y() > DOWN_VELOCITY)
    {
      if (dist.y < -VERT_OFFSET)
      {
        set_state(CHASING_ACCEL1);
      }
      else
      {
        set_state(CHASING_DOWN);
      }
    }
    break;
  case STUNNED:
    if (m_sprite->animation_done())
    {
      set_state(INVISIBLE);
    }
    break;
  case INVISIBLE:
    if (m_respawn_timer.check())
    {
      set_state(RECOVERING);
    }
    break;
  case RECOVERING:
    if (m_sprite->animation_done())
    {
      set_state(ROAMING_DOWN);
    }
    break;
  }

  m_dir = new_dir;
}

void
Ghoul::update_speed(const Vector& dist)
{
  const float vx = m_physic.get_velocity_x();
  if (vx >= -HORZ_SPEED && vx <= HORZ_SPEED)
  {
    m_physic.set_acceleration_x(0.0f);
    const float vy = dist.y < 0.0f ? (UP_VELOCITY + DOWN_VELOCITY) / 2.0f : DOWN_VELOCITY;
    const float t = dist.y / vy;
    if (t * HORZ_SPEED > std::abs(dist.x))
    {
      m_physic.set_velocity_x(dist.x / t);
    }
    else
    {
      m_physic.set_velocity_x(dist.x < 0.0f ? -HORZ_SPEED : HORZ_SPEED);
    }
  }
}

void
Ghoul::draw(DrawingContext& context)
{
  if (m_state != INVISIBLE)
    BadGuy::draw(context);
}

void
Ghoul::horizontal_thrust()
{
  const float vy = (UP_VELOCITY + DOWN_VELOCITY) / 2.0f;
  const Vector dist = to_target();
  const float t = dist.y / vy;
  if (t * (std::abs(m_physic.get_velocity_x()) + HORZ_SPEED) / 2.0f > std::abs(dist.x) || dist.y > 0)
    return; //no need for acceleration

  const float a = dist.x > 0.0f ? -HORZ_ACCELERATION : HORZ_ACCELERATION;
  m_physic.set_acceleration_x(a);

  const float vx = m_physic.get_velocity_x();
  const float vx_diff = HORZ_SPEED * HORZ_TRUST_MULTIPLIER;

  if (t == 0.0f)
  {
    m_physic.set_velocity_x(dist.x > 0.0f ? vx - vx_diff : vx + vx_diff);
  }
  else
  {
    const float vx_needed = dist.x / t - a * t / 2.0f;
    m_physic.set_velocity_x(std::clamp(vx_needed, vx - vx_diff, vx + vx_diff));
  }
}

void
Ghoul::start_roaming_decel()
{
  const float vx = m_physic.get_velocity_x();
  if (vx > 0)
  {
    m_physic.set_acceleration_x(-HORZ_ACCELERATION);
  }
  else if (vx < 0)
  {
    m_physic.set_acceleration_x(HORZ_ACCELERATION);
  }
}

void
Ghoul::roaming_decel_check()
{
  const float vx = m_physic.get_velocity_x();
  const float ax = m_physic.get_acceleration_x();
  if (vx * ax > 0)
  {
    m_physic.set_velocity_x(0.0f);
    m_physic.set_acceleration_x(0.0f);
  }
}

void
Ghoul::set_state(GhoulState new_state)
{
  if (m_state == new_state)
    return;

  switch (new_state)
  {
  case ROAMING_DOWN:
    start_roaming_decel();
    [[fallthrough]];
  case CHASING_DOWN:
    set_colgroup_active(COLGROUP_TOUCHABLE);
    m_physic.set_acceleration_y(0.0f);
    m_physic.set_velocity_y(DOWN_VELOCITY);
    set_action(m_dir == Direction::LEFT ? "left" : "right");
    break;
  case ROAMING_ACCEL1:
  case CHASING_ACCEL1:
    m_physic.set_velocity(0.f, 0.f);
    set_action(m_dir == Direction::LEFT ? "accel1-left" : "accel1-right", 1);
    break;
  case CHASING_ACCEL2:
    horizontal_thrust();
    [[fallthrough]];
  case ROAMING_ACCEL2:
    set_action(m_dir == Direction::LEFT ? "accel2-left" : "accel2-right", 1);
    m_physic.set_acceleration_y(UP_ACCELERATION);
    m_physic.set_velocity_y(UP_VELOCITY);
    break;
  case ROAMING_UP:
    start_roaming_decel();
    [[fallthrough]];
  case CHASING_UP:
    set_action(m_dir == Direction::LEFT ? "left-up" : "right-up");
    break;
  case STUNNED:
    SoundManager::current()->play("sounds/ghoul_stunned.ogg", get_pos());
    set_action(m_dir == Direction::LEFT ? "stunned-left" : "stunned-right", 1);
    m_physic.set_velocity(0.f, 0.f);
    m_physic.set_acceleration(0.f, 0.f);
    set_colgroup_active(COLGROUP_DISABLED);
    break;
  case INVISIBLE:
    m_respawn_timer.start(RESPAWN_TIME);
    m_home_pos = get_pos();
    break;
  case RECOVERING:
    SoundManager::current()->play("sounds/ghoul_recovering.ogg", get_pos());
    set_action(m_dir == Direction::LEFT ? "recovering-left" : "recovering-right", 1);
    break;
  }
  m_state = new_state;
}

HitResponse
Ghoul::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  BadGuy::collision_badguy(badguy, hit);
  return CONTINUE;
}

void
Ghoul::collision_solid(const CollisionHit& hit)
{
  // allows it to continue moving if it hits a wall.
}

void
Ghoul::collision_tile(uint32_t tile_attributes)
{
  // don't give it any unique tile interactions, such as hurting on spikes or swimming in water.
}


ObjectSettings
Ghoul::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Track Range"), &m_track_range, "track-range", DEFAULT_TRACK_RANGE);

  result.reorder({ "track-range", "speed", "direction", "x", "y" });

  return result;
}

bool
Ghoul::collision_squished(MovingObject& object)
{
  auto player = Sector::get().get_nearest_player(m_col.m_bbox);
  if (player)
    player->bounce(*this);

  kill_fall();
  return true;
}

void
Ghoul::kill_fall()
{
  // "killing" the Ghoul doesn't actually kill it, because it is going to respawn, but it pretends to die.
  set_state(STUNNED);
}
