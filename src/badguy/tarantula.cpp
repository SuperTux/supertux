//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "badguy/tarantula.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"

static const float FLYTIME = 1.2f;
static const float MOVE_SPEED = -100.0f;

Tarantula::Tarantula(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/tarantula/tarantula.sprite"),
  mode(),
  timer()
{
  m_physic.enable_gravity(false);
}

void
Tarantula::initialize()
{
  set_action(m_dir);
  mode = FLY_UP;
  m_physic.set_velocity_y(MOVE_SPEED);
  timer.start(FLYTIME/2);
}

bool
Tarantula::collision_squished(GameObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
Tarantula::collision_solid(const CollisionHit& hit)
{
  if (hit.top || hit.bottom) { // Hit floor or roof?
    m_physic.set_velocity_y(0);
  }
  if (m_frozen)
    BadGuy::collision_solid(hit);
}

void
Tarantula::active_update(float dt_sec)
{
  if (m_frozen)
  {
    BadGuy::active_update(dt_sec);
    return;
  }
  if (timer.check()) {
    if (mode == FLY_UP) {
      mode = FLY_DOWN;
      m_physic.set_velocity_y(-MOVE_SPEED);
    } else if (mode == FLY_DOWN) {
      mode = FLY_UP;
      m_physic.set_velocity_y(MOVE_SPEED);
    }
    timer.start(FLYTIME);
  }
  m_col.set_movement(m_physic.get_movement(dt_sec));

  auto player = get_nearest_player();
  if (player) {
    m_dir = (player->get_pos().x > get_pos().x) ? Direction::RIGHT : Direction::LEFT;
    set_action(m_dir);
  }
}

void
Tarantula::freeze()
{
  m_physic.enable_gravity(true);
  BadGuy::freeze();
}

void
Tarantula::unfreeze(bool melt)
{
  BadGuy::unfreeze(melt);
  m_physic.enable_gravity(false);
  initialize();
}

bool
Tarantula::is_freezable() const
{
  return true;
}

std::vector<Direction>
Tarantula::get_allowed_directions() const
{
  return {};
}

/* EOF */
