//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "badguy/fish_swimming.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader_mapping.hpp"

static const float FISH_BEACH_TIME = 5.f;
static const float FISH_FLOAT_TIME = 2.f;

FishSwimming::FishSwimming(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/fish/ice/bluefish.sprite"),
  m_state(FishYState::BALANCED),
  m_beached_timer(),
  m_float_timer(),
  m_radius()
{
  parse_type(reader);
  reader.get("radius", m_radius, 100.0f);
}

FishSwimming::FishSwimming(const ReaderMapping& reader, const std::string& spritename) :
  BadGuy(reader, spritename),
  m_state(FishYState::BALANCED),
  m_beached_timer(),
  m_float_timer(),
  m_radius()
{
  reader.get("radius", m_radius, 100.0f);
}

GameObjectTypes
FishSwimming::get_types() const
{
  return {
    { "snow", _("Snow") },
    { "forest", _("Forest") }
  };
}

std::string
FishSwimming::get_default_sprite_name() const
{
  switch (m_type)
  {
    case FOREST:
      return "images/creatures/fish/forest/bluefish.sprite";
    default:
      return m_default_sprite_name;
  }
}

ObjectSettings
FishSwimming::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Radius"), &m_radius, "radius", 100.0f);

  result.reorder({ "radius", "direction", "x", "y" });

  return result;
}

void
FishSwimming::initialize()
{
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -128.f : 128.f);
  set_action("swim", m_dir);
  m_state = FishYState::BALANCED;
}

void
FishSwimming::collision_solid(const CollisionHit& hit)
{
  if (m_frozen) {
    BadGuy::collision_solid(hit);
  }
  else
  {
    if (m_in_water)
    {
      if (hit.left || hit.right)
        turn_around();
    }
    else
    {
      if (hit.bottom && !m_frozen)
      {
        m_physic.set_velocity(0.f, -300.f);
        if (!m_beached_timer.started())
          m_beached_timer.start(FISH_BEACH_TIME);
      }
    }
  }
}

HitResponse
FishSwimming::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (m_beached_timer.started())
     collision_solid(hit);

  if (!m_frozen && !m_beached_timer.started() &&
    ((hit.left && (m_dir == Direction::LEFT)) || (hit.right && (m_dir == Direction::RIGHT))))
    turn_around();

  BadGuy::collision_badguy(badguy, hit);
  return CONTINUE;
}

void
FishSwimming::update(float dt_sec)
{
  // Don't allow dying by going below the sector.
  if (BadGuy::get_state() != STATE_FALLING && !m_frozen &&
      m_in_water && get_bbox().get_bottom() >= Sector::get().get_height())
  {
    set_pos(Vector(get_bbox().get_left(),
                   Sector::get().get_height() - m_col.m_bbox.get_height()));
  }
  BadGuy::update(dt_sec);
}

void
FishSwimming::active_update(float dt_sec) {
  // Perform basic updates.
  BadGuy::active_update(dt_sec);
  m_in_water = !Sector::get().is_free_of_tiles(get_bbox(), true, Tile::WATER);
  m_physic.enable_gravity((!m_frozen && m_in_water) ? false : true);

  // Handle beached state when the fish is in water and beached_timer is active.
  if (m_in_water && m_beached_timer.started())
    m_beached_timer.stop();

  if (m_beached_timer.check())
  {
    ignite();
    m_physic.reset();
    m_beached_timer.stop();
  }

  // Handle y-velocity related functionality.
  if (!m_float_timer.started())
    m_float_timer.start(FISH_FLOAT_TIME);

  if (!m_in_water)
    m_state = FishYState::DISRUPTED;

  if (m_in_water && !m_frozen)
  {
    if (m_state == FishYState::DISRUPTED)
    {
      if (std::abs(m_physic.get_velocity_y()) >= 5.f) {
        m_physic.set_velocity_y(m_physic.get_velocity_y() / 1.25f);
      }
      else
      {
        m_state = FishYState::BALANCED;
        m_physic.set_velocity_y(0.f);
        m_start_position.y = get_pos().y;
      }
    }
    else
    {
      float yspeed = 10.f;
      if (m_float_timer.get_timeleft() <= 1.f)
      {
        yspeed = yspeed * -1.f;
      }
      m_physic.set_velocity_y(yspeed);
    }
  }

  if (!m_beached_timer.started())
  {
    // Handle x-velocity related functionality.
    float goal_x_velocity = m_dir == Direction::LEFT ? -128.f : 128.f;
    if (m_dir != Direction::LEFT && get_pos().x > (m_start_position.x + m_radius - 20.f))
      goal_x_velocity = -128.f;
    if (m_dir != Direction::RIGHT && get_pos().x < (m_start_position.x - m_radius + 20.f))
      goal_x_velocity = 128.f;

    maintain_velocity(goal_x_velocity);
  }
}

void
FishSwimming::freeze()
{
  BadGuy::freeze();
  if (m_beached_timer.started())
    m_beached_timer.stop();
}

void
FishSwimming::unfreeze(bool melt)
{
  BadGuy::unfreeze();
  initialize();
}

bool
FishSwimming::is_freezable() const
{
  return true;
}

void
FishSwimming::turn_around()
{
  if (m_frozen)
    return;

  m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
  set_action("swim", m_dir);
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -128.f : 128.f);
}

void
FishSwimming::maintain_velocity(float goal_x_velocity)
{
  if (m_frozen || !m_in_water)
    return;

  float current_x_velocity = m_physic.get_velocity_x();
  /* We're very close to our target speed. Just set it to avoid oscillation. */
  if ((current_x_velocity > (goal_x_velocity - 5.0f)) &&
    (current_x_velocity < (goal_x_velocity + 5.0f)))
  {
    m_physic.set_velocity_x(goal_x_velocity);
    m_physic.set_acceleration_x(0.0);
  }
  /* Check if we're going too slow or even in the wrong direction. */
  else if (((goal_x_velocity <= 0.0f) && (current_x_velocity > goal_x_velocity)) ||
    ((goal_x_velocity > 0.0f) && (current_x_velocity < goal_x_velocity))) {
    m_physic.set_acceleration_x(goal_x_velocity);
  }
  /* Check if we're going too fast. */
  else if (((goal_x_velocity <= 0.0f) && (current_x_velocity < goal_x_velocity)) ||
    ((goal_x_velocity > 0.0f) && (current_x_velocity > goal_x_velocity))) {
    m_physic.set_acceleration_x((-1.f) * goal_x_velocity);
  }
  else {
    /* The above should have covered all cases. */
    assert(false);
  }

  if ((m_dir == Direction::LEFT) && (m_physic.get_velocity_x() > 0.0f))
  {
    m_dir = Direction::RIGHT;
    set_action("swim-right", -1);
  }
  else if ((m_dir == Direction::RIGHT) && (m_physic.get_velocity_x() < 0.0f))
  {
    m_dir = Direction::LEFT;
    set_action("swim-left", -1);
  }
}

/* EOF */
