//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

//#include <config.h>

#include "badguy/captainsnowball.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

namespace{
  static const float WALK_SPEED = 100; 
  static const float BOARDING_SPEED = 200;
}


CaptainSnowball::CaptainSnowball(const lisp::Lisp& reader)
    : WalkingBadguy(reader, "images/creatures/snowball/cpt-snowball.sprite", "left", "right")
{
  walk_speed = BOARDING_SPEED;
  max_drop_height = -1;
  physic.set_velocity_y(-400);
}

CaptainSnowball::CaptainSnowball(const Vector& pos, Direction d)
    : WalkingBadguy(pos, d, "images/creatures/snowball/cpt-snowball.sprite", "left", "right")
{
  // Created during game eg. by dispencer. Board the enemy!
  walk_speed = BOARDING_SPEED;
  max_drop_height = -1;
  physic.set_velocity_y(-400);
}

bool
CaptainSnowball::might_climb(int width, int height)
{
  // make sure we check for at least a 1-pixel climb
  assert(height > 0);

  float x1;
  float x2;
  float y1a = bbox.p1.y + 1;
  float y2a = bbox.p2.y - 1;
  float y1b = bbox.p1.y + 1 - height;
  float y2b = bbox.p2.y - 1 - height;
  if (dir == LEFT) {
    x1 = bbox.p1.x - width;
    x2 = bbox.p1.x - 1;
  } else {
    x1 = bbox.p2.x + 1;
    x2 = bbox.p2.x + width;
  }
  return ((!Sector::current()->is_free_of_statics(Rect(x1, y1a, x2, y2a))) && (Sector::current()->is_free_of_statics(Rect(x1, y1b, x2, y2b))));
}

void
CaptainSnowball::active_update(float elapsed_time)
{
  if (on_ground() && might_climb(8, 64)) {
    physic.set_velocity_y(-400);
  } else if (on_ground() && might_fall(16)) {
    physic.set_velocity_y(-400);
    walk_speed = BOARDING_SPEED;
    physic.set_velocity_x(dir == LEFT ? -walk_speed : walk_speed);
  }
  WalkingBadguy::active_update(elapsed_time);
}

void
CaptainSnowball::collision_solid(const CollisionHit& hit)
{
  if (is_active() && (walk_speed == BOARDING_SPEED)) {
    walk_speed = WALK_SPEED;
    physic.set_velocity_x(dir == LEFT ? -walk_speed : walk_speed);
  }
  WalkingBadguy::collision_solid(hit);
}

bool
CaptainSnowball::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

IMPLEMENT_FACTORY(CaptainSnowball, "captainsnowball");
