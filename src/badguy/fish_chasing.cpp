//  SuperTux
//  Copyright (C) 2022 Daniel Ward <weluvgoatz@gmail.com>
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

#include "badguy/fish_chasing.hpp"

#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader_mapping.hpp"

static const float TRACK_DISTANCE = 200.0f;
static const float LOST_DISTANCE = 400.0f;
static const float CHASE_SPEED = 300.0f;
static const float REALIZATION_TIME = 0.7f;

FishChasing::FishChasing(const ReaderMapping& reader) :
  FishSwimming(reader, "images/creatures/fish/ice/dopefish.sprite"),
  m_chase_state(ChaseState::NORMAL),
  m_realization_timer(),
  m_track_distance(),
  m_lost_distance(),
  m_chase_speed()
{
  reader.get("track-distance", m_track_distance, TRACK_DISTANCE);
  reader.get("lost-distance", m_lost_distance, LOST_DISTANCE);
  reader.get("chase-speed", m_chase_speed, CHASE_SPEED);
}

void
FishChasing::active_update(float dt_sec) {
  //basic stuff
  BadGuy::active_update(dt_sec);
  m_in_water = !Sector::get().is_free_of_tiles(get_bbox(), true, Tile::WATER);
  m_physic.enable_gravity((!m_frozen && m_in_water) ? false : true);

  //beached stuff
  if (m_in_water && m_beached_timer.started())
    m_beached_timer.stop();

  if (m_beached_timer.check())
  {
    ignite();
    m_physic.reset();
    m_beached_timer.stop();
  }

  //behavior
  auto player = get_nearest_player();
  if (!player) return;
  Vector p1 = m_col.m_bbox.get_middle();
  Vector p2 = player->get_bbox().get_middle();
  Vector dist = (p2 - p1);

  switch (m_chase_state)
  {
  case NORMAL:
    FishSwimming::active_update(dt_sec);

    if (glm::length(dist) <= m_track_distance && m_in_water && !m_frozen)
    {
      m_realization_timer.start(REALIZATION_TIME);
      m_chase_state = ChaseState::FOUND;
    }
    break;
  case FOUND:
    m_sprite->set_action(m_dir == Direction::LEFT ? "notice-left" : "notice-right", 1);

    if (std::abs(glm::length(m_physic.get_velocity())) >= 1.f) {
      m_physic.set_velocity(m_physic.get_velocity() / 1.25f);
    }

    if (m_realization_timer.check())
    {
      m_realization_timer.stop();
      m_chase_state = ChaseState::CHASING;
    }
    break;
  case CHASING:
    if (glm::length(dist) > m_lost_distance)
    {
      m_realization_timer.start(REALIZATION_TIME);
      m_chase_state = ChaseState::LOST;
    }
    else if (glm::length(dist) >= 1 && m_in_water && !m_frozen)
    {
      m_dir = (m_physic.get_velocity_x() <= 0.f ? Direction::LEFT : Direction::RIGHT);
      m_sprite->set_action(m_dir == Direction::LEFT ? "chase-left" : "chase-right");
      Vector dir = glm::normalize(dist);
      m_physic.set_velocity(dir * m_chase_speed);
    }
    else
    {
      /* We somehow landed right on top of the player without colliding.
       * Sit tight and avoid a division by zero. */
    }
    break;
  case LOST:
    m_sprite->set_action(m_dir == Direction::LEFT ? "swim-left" : "swim-right");

    if (m_in_water && !m_frozen)
    {
      if (std::abs(glm::length(m_physic.get_velocity())) >= 10.f) {
        m_physic.set_velocity(m_physic.get_velocity() / 1.25f);
      }
      else if (std::abs(glm::length(m_physic.get_velocity())) < 10.f && m_realization_timer.check())
      {
        m_realization_timer.stop();
        m_physic.set_velocity(0.f, 0.f);
        m_start_position = get_pos();
        m_state = FishYState::BALANCED;
        m_chase_state = ChaseState::NORMAL;
      }
    }
    break;
  }
}

ObjectSettings
FishChasing::get_settings()
{
  ObjectSettings result = FishSwimming::get_settings();

  result.add_float(_("Tracking Distance"), &m_track_distance, "track-distance", m_track_distance);
  result.add_float(_("Losing Distance"), &m_lost_distance, "lost-distance", m_lost_distance);
  result.add_float(_("Chase Speed"), &m_chase_speed, "chase-speed", m_chase_speed);

  result.reorder({ "radius", "direction", "track-distance", "lost-distance", "chase-speed", "x", "y" });

  return result;
}

/* EOF */
