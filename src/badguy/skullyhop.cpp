//  SkullyHop - A Hopping Skull
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

#include "badguy/skullyhop.hpp"

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"

namespace {
static const std::string SKULLYHOP_SOUND = "sounds/hop.ogg";
}

SkullyHop::SkullyHop(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/skullyhop/skullyhop.sprite"),
  recover_timer(),
  state()
{
  SoundManager::current()->preload( SKULLYHOP_SOUND );
}

void
SkullyHop::initialize()
{
  // initial state is JUMPING, because we might start airborne
  state = JUMPING;
  m_sprite->set_action("jumping", m_dir);
}

void
SkullyHop::set_state(SkullyHopState newState)
{
  if (newState == STANDING) {
    m_physic.set_velocity_x(0);
    m_physic.set_velocity_y(0);
    m_sprite->set_action("standing", m_dir);

    recover_timer.start(0.5);
  } else
    if (newState == CHARGING) {
      m_sprite->set_action("charging", m_dir, 1);
    } else
      if (newState == JUMPING) {
        m_sprite->set_action("jumping", m_dir);
const float HORIZONTAL_SPEED = 220; /**< x-speed when jumping */
        m_physic.set_velocity_x(m_dir == Direction::LEFT ? -HORIZONTAL_SPEED : HORIZONTAL_SPEED);
const float VERTICAL_SPEED = -450;   /**< y-speed when jumping */
        m_physic.set_velocity_y(VERTICAL_SPEED);
        SoundManager::current()->play( SKULLYHOP_SOUND, get_pos());
      }

  state = newState;
}

bool
SkullyHop::collision_squished(GameObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  m_sprite->set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
SkullyHop::collision_solid(const CollisionHit& hit)
{
  if (m_frozen || BadGuy::get_state() == STATE_BURNING)
  {
    BadGuy::collision_solid(hit);
    return;
  }

  // just default behaviour (i.e. stop at floor/walls) when squished
  if (BadGuy::get_state() == STATE_SQUISHED) {
    BadGuy::collision_solid(hit);
  }

  // ignore collisions while standing still
  if (state != JUMPING)
    return;

  // check if we hit the floor while falling
  if (hit.bottom && m_physic.get_velocity_y() > 0 ) {
    set_state(STANDING);
  }
  // check if we hit the roof while climbing
  if (hit.top) {
    m_physic.set_velocity_y(0);
  }

  // check if we hit left or right while moving in either direction
  if (hit.left || hit.right) {
    m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
    m_sprite->set_action("jumping", m_dir);
    m_physic.set_velocity_x(-0.25f*m_physic.get_velocity_x());
  }
}

HitResponse
SkullyHop::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  // behaviour for badguy collisions is the same as for collisions with solids
  collision_solid(hit);

  return CONTINUE;
}

void
SkullyHop::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);

  // no change if frozen
  if (m_frozen)
    return;

  // charge when fully recovered
  if ((state == STANDING) && (recover_timer.check())) {
    set_state(CHARGING);
    return;
  }

  // jump as soon as charging animation completed
  if ((state == CHARGING) && (m_sprite->animation_done())) {
    set_state(JUMPING);
    return;
  }
}

void
SkullyHop::unfreeze(bool melt)
{
  BadGuy::unfreeze(melt);
  initialize();
}

bool
SkullyHop::is_freezable() const
{
  return true;
}

/* EOF */
