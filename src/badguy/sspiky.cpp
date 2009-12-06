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

#include "badguy/sspiky.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

static const float WALKSPEED = 80;

SSpiky::SSpiky(const Reader& reader)
  : WalkingBadguy(reader, "images/creatures/spiky/sleepingspiky.sprite", "left", "right"), state(SSPIKY_SLEEPING)
{
  walk_speed = WALKSPEED;
  max_drop_height = -1;
}

void
SSpiky::initialize()
{
  state = SSPIKY_SLEEPING;
  physic.set_velocity_x(0);
  sprite->set_action(dir == LEFT ? "sleeping-left" : "sleeping-right");
}

void
SSpiky::collision_solid(const CollisionHit& hit)
{
  if(state != SSPIKY_WALKING) {
    BadGuy::collision_solid(hit);
    return;
  }
  WalkingBadguy::collision_solid(hit);
}

HitResponse
SSpiky::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if(state != SSPIKY_WALKING) {
    return BadGuy::collision_badguy(badguy, hit);
  }
  return WalkingBadguy::collision_badguy(badguy, hit);
}

void
SSpiky::active_update(float elapsed_time) {

  if(state == SSPIKY_WALKING) {
    WalkingBadguy::active_update(elapsed_time);
    return;
  }

  if(state == SSPIKY_SLEEPING) {

    Player* player = this->get_nearest_player();
    if (player) {
      Rectf mb = this->get_bbox();
      Rectf pb = player->get_bbox();

      bool inReach_left = (pb.p2.x >= mb.p2.x-((dir == LEFT) ? 256 : 0));
      bool inReach_right = (pb.p1.x <= mb.p1.x+((dir == RIGHT) ? 256 : 0));
      bool inReach_top = (pb.p2.y >= mb.p1.y);
      bool inReach_bottom = (pb.p1.y <= mb.p2.y);

      if (inReach_left && inReach_right && inReach_top && inReach_bottom) {
        // wake up
        sprite->set_action(dir == LEFT ? "waking-left" : "waking-right", 1);
        state = SSPIKY_WAKING;
      }
    }

    BadGuy::active_update(elapsed_time);
  }

  if(state == SSPIKY_WAKING) {
    if(sprite->animation_done()) {
      // start walking
      state = SSPIKY_WALKING;
      WalkingBadguy::initialize();
    }

    BadGuy::active_update(elapsed_time);
  }
}

void
SSpiky::freeze()
{
  WalkingBadguy::freeze();
  sprite->set_action(dir == LEFT ? "iced-left" : "iced-right");
  state = SSPIKY_WALKING; // if we get hit while sleeping, wake up :)
}

bool
SSpiky::is_freezable() const
{
  return true;
}

/* EOF */
