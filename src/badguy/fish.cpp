//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include "fish.hpp"
#include "object/tilemap.hpp"

static const float FISH_JUMP_POWER = 700;
static const float FISH_FALL_BY_Y = 10;
static const float FISH_WAIT_TIME = 3;

Fish::Fish(const lisp::Lisp& reader)
    : waiting(false)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("fish");
  physic.enable_gravity(true);
}

Fish::Fish(float pos_x, float pos_y)
    : waiting(false)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("fish");
  physic.enable_gravity(true);
}

void
Fish::write(lisp::Writer& writer)
{
  writer.start_list("fish");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("fish");
}

HitResponse
Fish::collision_solid(GameObject& other, const CollisionHit& chit)
{
  return hit(chit);
}

HitResponse
Fish::collision_badguy(BadGuy& other, const CollisionHit& chit)
{
  return hit(chit);
}

HitResponse
Fish::hit(const CollisionHit& chit)
{
  if(chit.normal.y < .5) { // hit ceiling
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Fish::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  // check state and modify accordingly
  if (!waiting && physic.get_velocity_y() < 0
               && (get_pos().y - start_position.y) >= FISH_FALL_BY_Y) // we fell far enough
  {
  	start_waiting();
  }
  else if (waiting)
  {
  	waiting_for += elapsed_time;
  	if (waiting_for >= FISH_WAIT_TIME) // we've been waiting long enough
  	{
  	  waiting = false;
  	  physic.set_velocity_y(FISH_JUMP_POWER);
  	  physic.enable_gravity(true);
  	}
  }
  
  // set sprite
  sprite->set_action(physic.get_velocity_y() > 0 ? "normal" : "down");
  
  // we can't afford flying out of the tilemap, 'cause the engine would remove us.
  if ((get_pos().y - 31.8) < 0) // too high, let us fall
  {
  	physic.set_velocity_y(0);
  	physic.enable_gravity(true);
  }
  else if (Sector::current() && // spares us from possible segfaults
          (get_pos().y - 31.8) > (Sector::current()->solids->get_height() * 32)) // too low, wait.
  {
  	start_waiting();
  }
}

void
Fish::start_waiting()
{
  waiting_for = 0;
  waiting = true;
  physic.enable_gravity(false);
  physic.set_velocity_y(0);
}

IMPLEMENT_FACTORY(Fish, "fish")
