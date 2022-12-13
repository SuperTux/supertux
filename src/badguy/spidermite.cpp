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

#include "badguy/spidermite.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const float DEFAULT_BOUNCE_DISTANCE = 100.f;

SpiderMite::SpiderMite(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/spidermite/spidermite.sprite"),
  mode(SpiderMiteMode::HIDING),
  bounce_distance(DEFAULT_BOUNCE_DISTANCE)
{
  reader.get("bounce-distance", bounce_distance, 100.0f);
  initialize();
}

void
SpiderMite::initialize()
{
  set_start_position(get_pos());
  m_sprite->set_action(m_dir);
  m_physic.enable_gravity(false);
  mode = SpiderMiteMode::HIDING;
}

bool
SpiderMite::collision_squished(GameObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  spawn_squish_particles();
  m_sprite->set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
SpiderMite::collision_solid(const CollisionHit& hit)
{
  if (hit.top) { // hit floor or roof?
    m_physic.set_velocity_y(0);
  }
  else if (hit.bottom && is_active()) {
    mode = SpiderMiteMode::BOUNCING_UP;
  }
  if (m_frozen)
    BadGuy::collision_solid(hit);
}

void
SpiderMite::active_update(float dt_sec)
{
  if (m_frozen)
  {
    BadGuy::active_update(dt_sec);
    return;
  }

  float dist = glm::length(m_start_position - get_pos());
  switch (mode) {
  case HIDING:
    m_physic.enable_gravity(false);
    m_col.set_movement(Vector(0.f, 0.f));
    break;
  case BOUNCING_DOWN:
    if ((get_pos().y - m_start_position.y) >= bounce_distance)
      mode = SpiderMiteMode::BOUNCING_UP;
    m_col.set_movement(Vector(0.f, 2.f * std::min((dt_sec + (dist * dt_sec * 4.f)), 300.f * dt_sec)));
    break;
  case BOUNCING_UP:
    m_col.set_movement(Vector(0.f, 2.f * std::max((dt_sec - (dist * dt_sec * 4.f)), -300.f * dt_sec)));

    if (dist <= 2.f)
    {
      mode = SpiderMiteMode::BOUNCING_DOWN;
    }
    break;
  }

  auto player = get_nearest_player();
  if (!player)
    return;
  if (player->get_bbox().get_left() <= get_bbox().get_right() + 16.f &&
    player->get_bbox().get_right() >= get_bbox().get_left() - 16.f &&
    mode == SpiderMiteMode::HIDING)
  {
    m_physic.enable_gravity(true);
    m_physic.reset();
    mode = SpiderMiteMode::BOUNCING_DOWN;
  }
}

ObjectSettings
SpiderMite::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Bounce Distance"), &bounce_distance, "bounce-distance", 100.0f);

  result.reorder({ "bounce-distance", "direction", "x", "y" });

  return result;
}

void
SpiderMite::freeze()
{
  m_physic.enable_gravity(true);
  BadGuy::freeze();
}

void
SpiderMite::unfreeze(bool melt)
{
  BadGuy::unfreeze(melt);
  m_physic.enable_gravity(false);
  initialize();
}

bool
SpiderMite::is_freezable() const
{
  return true;
}

/* EOF */
