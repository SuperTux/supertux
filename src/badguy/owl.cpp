//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "badguy/owl.hpp"

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

#define FLYING_SPEED 120.0

Owl::Owl(const Reader& reader) :
  BadGuy(reader, "images/creatures/owl/owl.sprite")
{
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

Owl::Owl(const Vector& pos, Direction d)
  : BadGuy(pos, d, "images/creatures/owl/owl.sprite")
{
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

void
Owl::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -FLYING_SPEED : FLYING_SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
Owl::collision_squished(GameObject&)
{
  kill_fall ();
  return true;
}

void
Owl::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    kill_fall ();
  }
}

HitResponse
Owl::collision_player(Player& player, const CollisionHit& hit)
{
  //Hack to tell if we should die
  HitResponse response = BadGuy::collision_player(player, hit);
  if(response == FORCE_MOVE) {
    kill_fall ();
  }

  return ABORT_MOVE;
}

/* EOF */
