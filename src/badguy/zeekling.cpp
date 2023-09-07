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

#include "math/easing.hpp"
#include "math/random.hpp"
#include "math/vector.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

Zeekling::Zeekling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"),
  speed(gameRandom.randf(130.0f, 171.0f)),
  m_easing_progress(0.0),
  m_swoop_up_timer(),
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
Zeekling::on_bump_horizontal()
{
  m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
  set_action(m_dir);
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);

  if (state == DIVING)
  {
    state = FLYING;
    m_physic.set_velocity_y(0);
  }
}

void
Zeekling::on_bump_vertical()
{
  if (BadGuy::get_state() == STATE_BURNING)
  {
    m_physic.set_velocity(0, 0);
    return;
  }

  switch (state) {
    case DIVING:
      state = CLIMBING;
      set_action(m_dir);
      break;

    case CLIMBING:
      state = FLYING;
      break;

    default:
      break;
  }

    m_physic.set_velocity_y(state == CLIMBING ? -speed : 0);
}

void
Zeekling::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
  {
    BadGuy::collision_solid(hit);
    return;
  }

  if (m_sprite->get_action() == "squished-left" ||
      m_sprite->get_action() == "squished-right")
  {
    return;
  }

  if (hit.top || hit.bottom) {
    on_bump_vertical();
  }
  else if (hit.left || hit.right)
  {
    on_bump_horizontal();
  }
}

bool
Zeekling::should_we_dive()
{
  if (m_frozen) return false;

  // Left/rightmost point of the hitbox.
  Vector eye;
  const Rectf& bbox = get_bbox().grown(1.f);
  eye = bbox.get_middle();
  eye.x = m_dir == Direction::LEFT ? bbox.get_left() : bbox.get_right();

  const Vector& plrmid = get_nearest_player()->get_bbox().get_middle();

  // Do not dive if we are not above the player.
  float height = plrmid.y - eye.y;
  if (height <= 0) return false;

  // Do not dive if we are too far above the player.
  if (height > 512) return false;

  const Vector& rangeend = {eye.x + ((plrmid.y - eye.y) *
                                     (m_dir == Direction::LEFT ? -1 : 1)),
                            plrmid.y};

  // FIXME: Give the actual object that hit the raycast, or at least the hitbox
  // to avoid having to raycast a bunch of times
  return !Sector::get().free_line_of_sight(eye, rangeend, false, this) &&
          Sector::get().can_see_player(eye);
}

void
Zeekling::active_update(float dt_sec) {
  switch (state) {
    case FLYING:
      if (state != DIVING && should_we_dive())
      {
        // swoop a bit up
        state = DIVING;
        m_physic.set_velocity_y(2 * -(fabsf(m_physic.get_velocity_x())));
        m_swoop_up_timer.start(0.2f);
        set_action("charge", m_dir);
      }

      break;

    case DIVING:
      if (!m_swoop_up_timer.check()) break;

      // swoop down
      set_action("dive", m_dir);
      m_physic.set_velocity_y(2 * fabsf(m_physic.get_velocity_x()));
      break;

    case CLIMBING:
      // Stop climbing when we're back at initial height.
      if (get_pos().y <= m_start_position.y) {
        state = FLYING;
        m_physic.set_velocity_y(0);
      }
      break;
  }

    BadGuy::active_update(dt_sec);
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
