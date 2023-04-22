//  Zeekling - flyer that swoops down when she spots the player
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

#include "badguy/zeekling.hpp"

#include <math.h>

#include "math/random.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"

Zeekling::Zeekling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"),
  speed(gameRandom.randf(130.0f, 171.0f)),
  diveRecoverTimer(),
  state(FLYING),
  last_player(nullptr),
  last_player_pos(0.0f, 0.0f),
  last_self_pos(0.0f, 0.0f)
{
  m_physic.enable_gravity(false);
}

void
Zeekling::initialize()
{
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
  set_action(m_dir);
}

bool
Zeekling::collision_squished(GameObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
Zeekling::onBumpHorizontal()
{
  if (state == FLYING) {
    m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
    set_action(m_dir);
    m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
  } else
    if (state == DIVING) {
      m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
      state = FLYING;
      set_action(m_dir);
      m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
      m_physic.set_velocity_y(0);
    } else
      if (state == CLIMBING) {
        m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
        set_action(m_dir);
        m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
      } else {
        assert(false);
      }
}

void
Zeekling::onBumpVertical()
{
  if (BadGuy::get_state() == STATE_BURNING)
  {
    m_physic.set_velocity_y(0);
    m_physic.set_velocity_x(0);
    return;
  }
  if (state == FLYING) {
    m_physic.set_velocity_y(0);
  } else
    if (state == DIVING) {
      state = CLIMBING;
      m_physic.set_velocity_y(-speed);
      set_action(m_dir);
    } else
      if (state == CLIMBING) {
        state = FLYING;
        m_physic.set_velocity_y(0);
      }
}

void
Zeekling::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
    BadGuy::collision_solid(hit);
  else
  {
    if (m_sprite->get_action() == "squished-left" ||
      m_sprite->get_action() == "squished-right")
    {
      return;
    }

    if (hit.top || hit.bottom) {
      onBumpVertical();
    }
    else if (hit.left || hit.right) {
      onBumpHorizontal();
    }
  }
}

/** linear prediction of player and badguy positions to decide if we should enter the DIVING state */
bool
Zeekling::should_we_dive()
{
  if (m_frozen)
    return false;

  const auto player = get_nearest_player();
  if (player && last_player && (player == last_player)) {

    // get positions, calculate movement
    const Vector& player_pos = player->get_pos();
    const Vector player_mov = (player_pos - last_player_pos);
    const Vector self_pos = m_col.m_bbox.p1();
    const Vector self_mov = (self_pos - last_self_pos);

    // new vertical speed to test with
    float vy = 2*fabsf(self_mov.x);

    // do not dive if we are not above the player
    float height = player_pos.y - self_pos.y;
    if (height <= 0) return false;

    // do not dive if we are too far above the player
    if (height > 512) return false;

    // do not dive if we would not descend faster than the player
    float relSpeed = vy - player_mov.y;
    if (relSpeed <= 0) return false;

    // guess number of frames to descend to same height as player
    float estFrames = height / relSpeed;

    // guess where the player would be at this time
    float estPx = (player_pos.x + (estFrames * player_mov.x));

    // guess where we would be at this time
    float estBx = (self_pos.x + (estFrames * self_mov.x));

    // near misses are OK, too
    if (fabsf(estPx - estBx) < 8) return true;
  }

  // update last player tracked, as well as our positions
  last_player = player;
  if (player) {
    last_player_pos = player->get_pos();
    last_self_pos = m_col.m_bbox.p1();
  }

  return false;
}

void
Zeekling::active_update(float dt_sec) {
  if (state == FLYING) {
    if (should_we_dive()) {
      state = DIVING;
      m_physic.set_velocity_y(2*fabsf(m_physic.get_velocity_x()));
      set_action("diving", m_dir);
    }
    BadGuy::active_update(dt_sec);
    return;
  } else if (state == DIVING) {
    BadGuy::active_update(dt_sec);
    return;
  } else if (state == CLIMBING) {
    // stop climbing when we're back at initial height
    if (get_pos().y <= m_start_position.y) {
      state = FLYING;
      m_physic.set_velocity_y(0);
    }
    BadGuy::active_update(dt_sec);
    return;
  } else {
    assert(false);
  }
}

void
Zeekling::freeze()
{
  BadGuy::freeze();
  m_physic.enable_gravity(true);
}

void
Zeekling::unfreeze(bool melt)
{
  BadGuy::unfreeze(melt);
  m_physic.enable_gravity(false);
  state = FLYING;
  initialize();
}

bool
Zeekling::is_freezable() const
{
  return true;
}

/* EOF */
