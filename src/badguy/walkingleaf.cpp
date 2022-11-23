//  SuperTux - Walking Leaf
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#include "badguy/walkingleaf.hpp"

#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

WalkingLeaf::WalkingLeaf(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/walkingleaf/walkingleaf.sprite", "left", "right")
{
  walk_speed = 60;
  max_drop_height = 16;
}

void
WalkingLeaf::active_update(float dt_sec)
{
  if (!m_frozen && !m_ignited)
  {
    Rectf floatbox = get_bbox();
    floatbox.set_bottom(get_bbox().get_bottom() + 32.f);
    bool float_here = (Sector::get().is_free_of_statics(floatbox));

    if (!float_here) {
      m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
    }
    else {
      m_sprite->set_action(m_dir == Direction::LEFT ? "float-left" : "float-right");
      if (m_physic.get_velocity_y() >= 20.f) {
        m_physic.set_velocity_y(20.f);
      }
    }
  }

  WalkingBadguy::active_update(dt_sec);
}

bool
WalkingLeaf::collision_squished(GameObject& object)
{
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

  m_sprite->set_action(m_dir == Direction::LEFT ? "squished-left" : "squished-right");
  // Spawn death particles
  spawn_explosion_sprites(3, "images/particles/walkingleaf.sprite");
  kill_squished(object);
  return true;
}

bool
WalkingLeaf::is_freezable() const
{
  return true;
}
/* EOF */
