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

#include "badguy/plant.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"

static const float PLANT_SPEED = 80;
static const float WAKE_TIME = .5;

Plant::Plant(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/plant/plant.sprite"),
  timer(),
  state(PLANT_SLEEPING)
{
}

void
Plant::initialize()
{
  //FIXME: turns plant around for debugging
  m_dir = m_dir == LEFT ? RIGHT : LEFT;

  state = PLANT_SLEEPING;
  m_physic.set_velocity_x(0);
  m_sprite->set_action(m_dir == LEFT ? "sleeping-left" : "sleeping-right");
}

void
Plant::collision_solid(const CollisionHit& hit)
{
  if (hit.top || hit.bottom) {
    m_physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    m_dir = m_dir == LEFT ? RIGHT : LEFT;
    m_sprite->set_action(m_dir == LEFT ? "left" : "right");
    m_physic.set_velocity_x(-m_physic.get_velocity_x());
  }
}

HitResponse
Plant::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if (state != PLANT_WALKING) return CONTINUE;

  if (hit.left || hit.right) {
    m_dir = m_dir == LEFT ? RIGHT : LEFT;
    m_sprite->set_action(m_dir == LEFT ? "left" : "right");
    m_physic.set_velocity_x(-m_physic.get_velocity_x());
  }

  return CONTINUE;
}

void
Plant::active_update(float dt_sec) {
  BadGuy::active_update(dt_sec);

  if (state == PLANT_SLEEPING) {

    auto player = get_nearest_player();
    if (player) {
      Rectf pb = player->get_bbox();

      bool inReach_left = (pb.p2.x >= m_col.m_bbox.p2.x-((m_dir == LEFT) ? 256 : 0));
      bool inReach_right = (pb.p1.x <= m_col.m_bbox.p1.x+((m_dir == RIGHT) ? 256 : 0));
      bool inReach_top = (pb.p2.y >= m_col.m_bbox.p2.y);
      bool inReach_bottom = (pb.p1.y <= m_col.m_bbox.p1.y);

      if (inReach_left && inReach_right && inReach_top && inReach_bottom) {
        // wake up
        m_sprite->set_action(m_dir == LEFT ? "waking-left" : "waking-right");
        if (!timer.started()) timer.start(WAKE_TIME);
        state = PLANT_WAKING;
      }
    }
  }

  if (state == PLANT_WAKING) {
    if (timer.check()) {
      // start walking
      m_sprite->set_action(m_dir == LEFT ? "left" : "right");
      m_physic.set_velocity_x(m_dir == LEFT ? -PLANT_SPEED : PLANT_SPEED);
      state = PLANT_WALKING;
    }
  }

}

void
Plant::ignite()
{
  BadGuy::ignite();
  if (state == PLANT_SLEEPING && m_sprite->has_action("sleeping-burning-left")) {
    m_sprite->set_action(m_dir == LEFT ? "sleeping-burning-left" : "sleeping-burning-right", 1);
  }
}
/* EOF */
