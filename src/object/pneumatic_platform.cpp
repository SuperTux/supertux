//  SuperTux - PneumaticPlatform
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "object/pneumatic_platform.hpp"

#include "object/player.hpp"
#include "object/portable.hpp"
#include "supertux/sector.hpp"

PneumaticPlatform::PneumaticPlatform(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/platforms/small.sprite", LAYER_OBJECTS, COLGROUP_STATIC),
  m_master(nullptr),
  m_slave(nullptr),
  m_start_y(0),
  m_offset_y(0),
  m_speed_y(0),
  m_contacts()
{
  m_start_y = get_pos().y;
}

PneumaticPlatform::PneumaticPlatform(PneumaticPlatform* master_) :
  MovingSprite(*master_),
  m_master(master_),
  m_slave(this),
  m_start_y(master_->m_start_y),
  m_offset_y(-master_->m_offset_y),
  m_speed_y(0),
  m_contacts()
{
  set_pos(get_pos() + Vector(m_master->get_bbox().get_width(), 0));
  m_master->m_master = m_master;
  m_master->m_slave = this;
}

PneumaticPlatform::~PneumaticPlatform()
{
  if ((this == m_master) && (m_master)) {
    m_slave->m_master = nullptr;
    m_slave->m_slave = nullptr;
  }
  if ((m_master) && (this == m_slave)) {
    m_master->m_master = nullptr;
    m_master->m_slave = nullptr;
  }
  m_master = nullptr;
  m_slave = nullptr;
}

HitResponse
PneumaticPlatform::collision(GameObject& other, const CollisionHit& )
{
  // somehow the hit parameter does not get filled in, so to determine (hit.top == true) we do this:
  auto mo = dynamic_cast<MovingObject*>(&other);
  if (!mo) return FORCE_MOVE;
  if ((mo->get_bbox().p2.y) > (m_bbox.p1.y + 2)) return FORCE_MOVE;

  auto pl = dynamic_cast<Player*>(mo);
  if (pl) {
    if (pl->is_big()) m_contacts.insert(nullptr);
    auto po = pl->get_grabbed_object();
    auto pomo = dynamic_cast<MovingObject*>(po);
    if (pomo) m_contacts.insert(pomo);
  }

  m_contacts.insert(&other);
  return FORCE_MOVE;
}

void
PneumaticPlatform::update(float dt_sec)
{
  if (!m_slave) {
    Sector::get().add<PneumaticPlatform>(this);
    return;
  }
  if (!m_master) {
    return;
  }
  if (this == m_slave) {
    m_offset_y = -m_master->m_offset_y;
    m_movement = Vector(0, (m_start_y + m_offset_y) - get_pos().y);
  }
  if (this == m_master) {
    int contact_diff = static_cast<int>(m_contacts.size()) - static_cast<int>(m_slave->m_contacts.size());
    m_contacts.clear();
    m_slave->m_contacts.clear();

    m_speed_y += (static_cast<float>(contact_diff) * dt_sec) * 12.8f;
    m_speed_y -= (m_offset_y * dt_sec * 0.05f);
    m_speed_y *= 1 - dt_sec;
    m_offset_y += m_speed_y * dt_sec * Sector::get().get_gravity();
    if (m_offset_y < -256) { m_offset_y = -256; m_speed_y = 0; }
    if (m_offset_y > 256) { m_offset_y = 256; m_speed_y = -0; }
    m_movement = Vector(0, (m_start_y + m_offset_y) - get_pos().y);
  }
}

void
PneumaticPlatform::move_to(const Vector& pos)
{
  Vector shift = pos - m_bbox.p1;
  if (this == m_slave) {
    m_master->set_pos(m_master->get_pos() + shift);
  } else if (this == m_master) {
    m_slave->set_pos(m_slave->get_pos() + shift);
  }
  MovingObject::move_to(pos);
  m_start_y += shift.y;
}

void
PneumaticPlatform::editor_delete()
{
  m_master->remove_me();
  m_slave->remove_me();
}

void
PneumaticPlatform::after_editor_set()
{
  MovingSprite::after_editor_set();
  m_slave->change_sprite(m_sprite_name);
}

/* EOF */
