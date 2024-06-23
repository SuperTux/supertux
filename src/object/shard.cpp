//  SuperTux
//  Copyright (C) 2021 Daniel Ward <weluvgoatz@gmail.com>
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

#include "object/shard.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Shard::Shard(const ReaderMapping& reader) :
  StickyObject(reader, "images/creatures/crystallo/shard.sprite", LAYER_TILES - 2, COLGROUP_MOVING),
  m_physic(),
  m_stick_timer()
{
  m_physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/crystallo-shardhit.ogg");
}

Shard::Shard(const Vector& pos, const Vector& velocity, const std::string& sprite) :
  StickyObject(pos, sprite, LAYER_TILES - 2, COLGROUP_MOVING),
  m_physic(),
  m_stick_timer()
{
  m_physic.enable_gravity(true);
  m_physic.set_velocity(velocity);
  set_action("default");
  SoundManager::current()->preload("sounds/crystallo-shardhit.ogg");
}

void
Shard::update(float dt_sec)
{
  m_sticky = true;

  if (m_physic.get_velocity() != Vector(0.f, 0.f) && !m_sticking)
    m_sprite->set_angle(math::degrees(math::angle(Vector(m_physic.get_velocity_x(), m_physic.get_velocity_y()))));
  if (m_stick_timer.check())
    remove_me();

  m_col.set_movement(m_physic.get_movement(dt_sec));

  StickyObject::update(dt_sec);
}

void
Shard::collision_solid(const CollisionHit& hit)
{
  m_physic.set_velocity(0.f, 0.f);
  m_physic.set_acceleration(0.f, 0.f);
  m_physic.enable_gravity(hit.bottom);
  if (!m_stick_timer.started())
  {
    m_stick_timer.start(5.f);
    SoundManager::current()->play("sounds/crystallo-shardhit.ogg", get_pos());
  }
}

HitResponse
Shard::collision(GameObject& other, const CollisionHit&)
{
  // ignore collisions with other shards
  auto shard = dynamic_cast<Shard*>(&other);
  if (&other == shard)
    return ABORT_MOVE;
  // kill badguys
  auto badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy != nullptr)
    badguy->kill_fall();
  // kill players
  auto player = dynamic_cast<Player*>(&other);
  if (player != nullptr)
    player->kill(false);
  return ABORT_MOVE;
}

/* EOF */
