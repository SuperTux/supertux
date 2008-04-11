//  $Id$
//
//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "captainsnowball.hpp"

namespace{
  static const float WALK_SPEED = 50; 
  static const float BOARDING_SPEED = 300;
}


CaptainSnowball::CaptainSnowball(const lisp::Lisp& reader)
    : WalkingBadguy(reader, "images/creatures/snowball/cpt-snowball.sprite", "left", "right")
{
  walk_speed = WALK_SPEED; // peg leg
  max_drop_height = -1;// eye patch
}

CaptainSnowball::CaptainSnowball(const Vector& pos, Direction d)
    : WalkingBadguy(pos, d, "images/creatures/snowball/cpt-snowball.sprite", "left", "right")
{
  // Created during game eg. by dispencer. Board the enemy!
  walk_speed = BOARDING_SPEED;
  physic.set_velocity_y(-500);
}

void
CaptainSnowball::collision_solid(const CollisionHit& hit)
{
  WalkingBadguy::collision_solid(hit);
  if( is_active() ){ 
    walk_speed = WALK_SPEED;
    physic.set_velocity_x(dir == LEFT ? -walk_speed : walk_speed);
  }
}

bool
CaptainSnowball::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

IMPLEMENT_FACTORY(CaptainSnowball, "captainsnowball")
