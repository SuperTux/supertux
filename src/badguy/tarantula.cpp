//  SuperTux
//  Copyright (C) 2024 MatusGuy <matusguy@supertuxproject.org>
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
static const float MOVE_SPEED = 100.0f;

Tarantula::Tarantula(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/tarantula/tarantula.sprite")
{
  parse_type(reader);

  m_physic.enable_gravity(false);
}

void
Tarantula::initialize()
{
  m_physic.set_velocity_x(MOVE_SPEED);
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
  BadGuy::collision_solid(hit);

  if (hit.bottom) {
    kill_fall();
  }

  if (hit.left) {
    m_physic.set_velocity_x(MOVE_SPEED);
  }

  if (hit.right) {
    m_physic.set_velocity_x(-MOVE_SPEED);
  }
}

void
Tarantula::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
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

GameObjectTypes
Tarantula::get_types() const
{
  return {
    { "tarantula", _("Tarantula") },
    { "spidermite", _("Spidermite") }
  };
}

std::string
Tarantula::get_default_sprite_name() const
{
  switch (m_type)
  {
    case TARANTULA: return "images/creatures/tarantula/tarantula.sprite";
    case SPIDERMITE: return "images/creatures/spidermite/spidermite.sprite";
  }
  return "images/creatures/tarantula/tarantula.sprite";
}

std::vector<Direction>
Tarantula::get_allowed_directions() const
{
  return {};
}

/* EOF */
