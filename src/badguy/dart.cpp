//  $Id: dart.cpp 3327 2006-04-13 15:02:40Z sommer $
//
//  Dart - Your average poison dart
//  Copyright (C) 2006 Christoph Sommer <supertux@2006.expires.deltadevelopment.de>
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

#include "dart.hpp"

namespace {
  const float SPEED = 200;
}

Dart::Dart(const lisp::Lisp& reader) : set_direction(false), parent(0)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(9, 1);
  sprite = sprite_manager->create("images/creatures/dart/dart.sprite");
  physic.enable_gravity(false);
  countMe = false;
}

Dart::Dart(float pos_x, float pos_y, Direction d, const BadGuy* parent = 0) : set_direction(true), initial_direction(d), parent(parent)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(9, 1);
  sprite = sprite_manager->create("images/creatures/dart/dart.sprite");
  physic.enable_gravity(false);
  countMe = false;
}

void
Dart::write(lisp::Writer& writer)
{
  writer.start_list("dart");
  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.end_list("dart");
}

void
Dart::activate()
{  
  if (set_direction) dir = initial_direction;
  physic.set_velocity_x(dir == LEFT ? -::SPEED : ::SPEED);
  sprite->set_action(dir == LEFT ? "flying-left" : "flying-right");
}

void
Dart::deactivate()
{  
  remove_me();
}

HitResponse 
Dart::collision_solid(GameObject& , const CollisionHit& )
{
  remove_me();
  return ABORT_MOVE;
}

HitResponse 
Dart::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  // ignore collisions with parent
  if (&badguy == parent) {
    return FORCE_MOVE;
  }
  remove_me();
  badguy.kill_fall();
  return ABORT_MOVE;
}

HitResponse 
Dart::collision_player(Player& player, const CollisionHit& hit)
{
  remove_me();
  return BadGuy::collision_player(player, hit);
}



IMPLEMENT_FACTORY(Dart, "dart")

