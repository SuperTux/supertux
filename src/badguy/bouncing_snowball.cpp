//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "bouncing_snowball.hpp"

static const float JUMPSPEED = 450;
static const float WALKSPEED = 80;

BouncingSnowball::BouncingSnowball(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/bouncing_snowball/bouncing_snowball.sprite");
  set_direction = false;
}

BouncingSnowball::BouncingSnowball(float pos_x, float pos_y, Direction d)
{
   start_position.x = pos_x;
   start_position.y = pos_y;
   bbox.set_size(31.8, 31.8);
   sprite = sprite_manager->create("images/creatures/bouncing_snowball/bouncing_snowball.sprite");
   set_direction = true;
   initial_direction = d;
}

void
BouncingSnowball::write(lisp::Writer& writer)
{
  writer.start_list("bouncingsnowball");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("bouncingsnowball");
}

void
BouncingSnowball::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
BouncingSnowball::collision_squished(Player& player)
{
  sprite->set_action("squished");
  kill_squished(player);
  return true;
}

HitResponse
BouncingSnowball::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(hit.normal.y < -.5) { // hit floor
    physic.set_velocity_y(JUMPSPEED);
  } else if(hit.normal.y > .5) { // bumped on roof
    physic.set_velocity_y(0);
  } else { // left or right collision
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

HitResponse
BouncingSnowball::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
if (dynamic_cast<BouncingSnowball*>(&badguy))
{
  if(fabsf(hit.normal.x) > .8) { // left/right?
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");    
    physic.set_velocity_x(-physic.get_velocity_x());
  } else if(hit.normal.y < -.8) { // ground
    physic.set_velocity_y(JUMPSPEED);
  }
}

  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(BouncingSnowball, "bouncingsnowball")

