//  $Id$
//
//  SkullyHop - A Hopping Skull
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

#include "skullyhop.hpp"
#include "random_generator.hpp"

namespace {
  const float VERTICAL_SPEED = -450;   /**< y-speed when jumping */
  const float HORIZONTAL_SPEED = 220; /**< x-speed when jumping */
  const float MIN_RECOVER_TIME = 0.1; /**< minimum time to stand still before starting a (new) jump */
  const float MAX_RECOVER_TIME = 1.0; /**< maximum time to stand still before starting a (new) jump */
}

SkullyHop::SkullyHop(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/skullyhop/skullyhop.sprite")
{
  has_initial_direction = false;
}

SkullyHop::SkullyHop(const Vector& pos, Direction d)
	: BadGuy(pos, "images/creatures/skullyhop/skullyhop.sprite")
{
  has_initial_direction = true;
  initial_direction = d;
}

void
SkullyHop::write(lisp::Writer& writer)
{
  writer.start_list("skullyhop");
  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.end_list("skullyhop");
}

void
SkullyHop::activate()
{
  if(has_initial_direction)
    dir = initial_direction;

  // initial state is JUMPING, because we might start airborne
  state = JUMPING;
  sprite->set_action(dir == LEFT ? "jumping-left" : "jumping-right");
}

void
SkullyHop::set_state(SkullyHopState newState)
{
  if (newState == STANDING) {
    physic.set_velocity_x(0);
    physic.set_velocity_y(0);
    sprite->set_action(dir == LEFT ? "standing-left" : "standing-right");

    float recover_time = systemRandom.randf(MIN_RECOVER_TIME,MAX_RECOVER_TIME);
    recover_timer.start(recover_time);
  } else
  if (newState == CHARGING) {
    sprite->set_action(dir == LEFT ? "charging-left" : "charging-right", 1);
  } else
  if (newState == JUMPING) {
    sprite->set_action(dir == LEFT ? "jumping-left" : "jumping-right");
    physic.set_velocity_x(dir == LEFT ? -HORIZONTAL_SPEED : HORIZONTAL_SPEED);
    physic.set_velocity_y(VERTICAL_SPEED);
  }

  state = newState;
}

bool
SkullyHop::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  return true;
}

void
SkullyHop::collision_solid(const CollisionHit& hit)
{
  // ignore collisions while standing still
  if(state != JUMPING)
    return;

  // check if we hit the floor while falling
  if(hit.bottom) {
    set_state(STANDING);
  }
  // check if we hit the roof while climbing
  if(hit.top) { 
    physic.set_velocity_y(0);
  }

  // check if we hit left or right while moving in either direction
  if(hit.left || hit.right) {
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "jumping-left" : "jumping-right");
    physic.set_velocity_x(-0.25*physic.get_velocity_x());
  }
}

HitResponse
SkullyHop::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  // behaviour for badguy collisions is the same as for collisions with solids
  collision_solid(hit);

  return CONTINUE;
}

void
SkullyHop::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  // charge when fully recovered
  if ((state == STANDING) && (recover_timer.check())) {
    set_state(CHARGING);
    return;
  }

  // jump as soon as charging animation completed
  if ((state == CHARGING) && (sprite->animation_done())) {
    set_state(JUMPING);
    return;
  } 
}

IMPLEMENT_FACTORY(SkullyHop, "skullyhop")
