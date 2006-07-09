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

#include "mrrocket.hpp"

static const float SPEED = 200;

MrRocket::MrRocket(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/mr_rocket/mr_rocket.sprite")
{
}

MrRocket::MrRocket(const Vector& pos, Direction d)
	: BadGuy(pos, d, "images/creatures/mr_rocket/mr_rocket.sprite")
{
}

void
MrRocket::write(lisp::Writer& writer)
{
  writer.start_list("mrrocket");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mrrocket");
}

void
MrRocket::activate()
{
  physic.set_velocity_x(dir == LEFT ? -SPEED : SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
MrRocket::active_update(float elapsed_time)
{
  if (collision_timer.check()) {
    Sector::current()->add_object(new RocketExplosion(get_pos(), dir));
    remove_me();
  }
  else if (!collision_timer.started()) {
     movement=physic.get_movement(elapsed_time);
     sprite->set_action(dir == LEFT ? "left" : "right");
  }
}

bool
MrRocket::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  kill_fall();
  return true;
}

void
MrRocket::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    sprite->set_action(dir == LEFT ? "collision-left" : "collision-right");
    physic.set_velocity_x(0);
    collision_timer.start(0.2, true);
  }
}

IMPLEMENT_FACTORY(MrRocket, "mrrocket")
