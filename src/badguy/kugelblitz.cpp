//  $Id: Kugelblitz.cpp 2654 2005-06-29 14:16:22Z wansti $
// 
//  SuperTux
//  Copyright (C) 2005 Marek Moeckel <wansti@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "kugelblitz.hpp"

static const float JUMPSPEED=600;
static const float Kugelblitz_MID_TOLERANCE=4;
static const float Kugelblitz_LOW_TOLERANCE=2;

Kugelblitz::Kugelblitz(const lisp::Lisp& reader)
    : groundhit_pos_set(false)
{
  reader.get("x", start_position.x);
  start_position.y = 0; //place above visible area
  bbox.set_size(63.8, 127.8);
  sprite = sprite_manager->create("kugelblitz");
  sprite->set_action("falling");
  physic.enable_gravity(true);
}

void
Kugelblitz::write(lisp::Writer& writer)
{
  writer.start_list("kugelblitz");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("kugelblitz");
}

void
Kugelblitz::activate()
{
  physic.set_velocity_y(-300);
}

HitResponse
Kugelblitz::collision_solid(GameObject& , const CollisionHit& chit)
{
  return hit(chit);
}

HitResponse
Kugelblitz::collision_badguy(BadGuy& other , const CollisionHit& chit)
{
  other.kill_fall();
  return hit(chit);
}

HitResponse
Kugelblitz::hit(const CollisionHit& chit)
{
  // hit floor?
  if(chit.normal.y < -.5) {
    if (!groundhit_pos_set)
    {
      pos_groundhit = get_pos();
      groundhit_pos_set = true;
    }
    bbox.set_size(63.8, 63.8);
    sprite->set_action("flying");
    physic.set_velocity_y(0);
    physic.set_velocity_x(100);

  } else if(chit.normal.y < .5) { // bumped on roof
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Kugelblitz::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);
}

IMPLEMENT_FACTORY(Kugelblitz, "kugelblitz")
