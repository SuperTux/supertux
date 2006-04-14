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

#include "poisonivy.hpp"

static const float WALKSPEED = 80;

PoisonIvy::PoisonIvy(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  stay_on_platform = false;
  reader.get("stay-on-platform", stay_on_platform);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/poison_ivy/poison_ivy.sprite");
  set_direction = false;
}

PoisonIvy::PoisonIvy(float pos_x, float pos_y, Direction d, bool stay_on_plat = false)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  stay_on_platform = stay_on_plat;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/poison_ivy/poison_ivy.sprite");
  set_direction = true;
  initial_direction = d;
}

void
PoisonIvy::write(lisp::Writer& writer)
{
  writer.start_list("poisonivy");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  if (stay_on_platform) writer.write_bool("stay-on-platform", true);

  writer.end_list("poisonivy");
}

void
PoisonIvy::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
PoisonIvy::active_update(float elapsed_time)
{
  if (stay_on_platform && may_fall_off_platform())
  {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

bool
PoisonIvy::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  return true;
}

HitResponse
PoisonIvy::collision_solid(GameObject& , const CollisionHit& hit)
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
PoisonIvy::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left or right hit
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());       
  }

  return CONTINUE;
}

IMPLEMENT_FACTORY(PoisonIvy, "poisonivy")
