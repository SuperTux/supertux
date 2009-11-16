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

#include "badguy/mrrocket.hpp"

#include "lisp/writer.hpp"
#include "object/explosion.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

static const float SPEED = 200;

MrRocket::MrRocket(const Reader& reader) :
  BadGuy(reader, "images/creatures/mr_rocket/mr_rocket.sprite"),
  collision_timer()
{
}

MrRocket::MrRocket(const Vector& pos, Direction d) :
  BadGuy(pos, d, "images/creatures/mr_rocket/mr_rocket.sprite"),
  collision_timer()
{
}

void
MrRocket::write(lisp::Writer& writer)
{
  writer.start_list("mrrocket");

  writer.write("x", start_position.x);
  writer.write("y", start_position.y);

  writer.end_list("mrrocket");
}

void
MrRocket::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -SPEED : SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
MrRocket::active_update(float elapsed_time)
{
  if (collision_timer.check()) {
    Sector::current()->add_object(new Explosion(get_bbox().get_middle()));
    remove_me();
  }
  else if (!collision_timer.started()) {
    movement=physic.get_movement(elapsed_time);
    sprite->set_action(dir == LEFT ? "left" : "right");
  }
}

bool
MrRocket::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
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
    collision_timer.start(0.2f, true);
  }
}

IMPLEMENT_FACTORY(MrRocket, "mrrocket");

/* EOF */
