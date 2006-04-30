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

#include "mrbomb.hpp"
#include "bomb.hpp"

static const float WALKSPEED = 80;

MrBomb::MrBomb(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  sprite = sprite_manager->create("images/creatures/mr_bomb/mr_bomb.sprite");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_direction = false;
}

MrBomb::MrBomb(float pos_x, float pos_y, Direction d)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  sprite = sprite_manager->create("images/creatures/mr_bomb/mr_bomb.sprite");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_direction = true;
  initial_direction = d;
}

void
MrBomb::write(lisp::Writer& writer)
{
  writer.start_list("mrbomb");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mrbomb");
}

void
MrBomb::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
MrBomb::active_update(float elapsed_time)
{
  if (might_fall())
  {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  BadGuy::active_update(elapsed_time);
}

bool
MrBomb::collision_squished(Player& player)
{
  remove_me();
  Sector::current()->add_object(new Bomb(get_pos(), dir));
  kill_squished(player);
  return true;
}

HitResponse
MrBomb::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  } else { // hit right or left
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

HitResponse
MrBomb::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left or right
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");    
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

void
MrBomb::kill_fall()
{
  remove_me();
  Bomb* bomb = new Bomb(get_pos(), dir);
  Sector::current()->add_object(bomb);
  bomb->explode();
}

IMPLEMENT_FACTORY(MrBomb, "mrbomb")
