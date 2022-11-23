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

#include "badguy/viciousivy.hpp"

#include <math.h>

#include "math/random.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"

ViciousIvy::ViciousIvy(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/vicious_ivy/vicious_ivy.sprite", "left", "right")
{
  walk_speed = 80;
}

ViciousIvy::ViciousIvy(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/vicious_ivy/vicious_ivy.sprite", "left", "right")
{
  walk_speed = 80;
}

bool
ViciousIvy::is_freezable() const
{
  return true;
}

void
ViciousIvy::active_update(float dt_sec)
{
  if (!m_frozen && !m_ignited)
  {
    Rectf floatbox = get_bbox();
    floatbox.set_bottom(get_bbox().get_bottom() + 8.f);
    bool float_here = (Sector::get().is_free_of_statics(floatbox));

    if (!float_here) {
      m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
    } else {
      m_sprite->set_action(m_dir == Direction::LEFT ? "float-left" : "float-right");
      if (m_physic.get_velocity_y() >= 35.f) {
        m_physic.set_velocity_y(35.f);
      }
    }
  }

  WalkingBadguy::active_update(dt_sec);
}

bool
ViciousIvy::collision_squished(GameObject& object)
{
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

  m_sprite->set_action(m_dir == Direction::LEFT ? "squished-left" : "squished-right");
  // Spawn death particles
  spawn_explosion_sprites(3, "images/particles/viciousivy.sprite");
  kill_squished(object);
  return true;
}

/* EOF */
