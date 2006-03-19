//  $Id$
// 
//  Zeekling - flyer that swoops down when she spots the player
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
#include <math.h>

#include "zeekling.hpp"

Zeekling::Zeekling(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/zeekling/zeekling.sprite");
  set_direction = false;
  state = FLYING;
}

Zeekling::Zeekling(float pos_x, float pos_y, Direction d)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(63.8, 50.8);
  sprite = sprite_manager->create("images/creatures/zeekling/zeekling.sprite");
  set_direction = true;
  initial_direction = d;
  state = FLYING;
}

void
Zeekling::write(lisp::Writer& writer)
{
  writer.start_list("zeekling");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("zeekling");
}

void
Zeekling::activate()
{
  speed = 130 + (rand() % 41);
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -speed : speed);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
Zeekling::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  kill_fall();
  return true;
}

void 
Zeekling::onBumpHorizontal() {
  if (state == FLYING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(dir == LEFT ? -speed : speed);
  } else
  if (state == DIVING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    state = FLYING;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(dir == LEFT ? -speed : speed);
    physic.set_velocity_y(0);
  } else
  if (state == CLIMBING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(dir == LEFT ? -speed : speed);
  }
}

void 
Zeekling::onBumpVertical() {
  if (state == FLYING) {
    physic.set_velocity_y(0);
  } else
  if (state == DIVING) {
    state = CLIMBING;
    physic.set_velocity_y(speed);
    sprite->set_action(dir == LEFT ? "left" : "right");
  } else
  if (state == CLIMBING) {
    state = FLYING;
    physic.set_velocity_y(0);
  }
}

HitResponse
Zeekling::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) {
    onBumpVertical(); 
  } else {
    onBumpHorizontal();
  }

  return CONTINUE;
}

/**
 * linear prediction of player and badguy positions to decide if we should enter the DIVING state
 */
bool 
Zeekling::should_we_dive() {
  const MovingObject* player = this->get_nearest_player();
  if (!player) return false;

  const MovingObject* badguy = this;

  const Vector playerPos = player->get_pos();
  const Vector playerMov = player->get_movement();

  const Vector badguyPos = badguy->get_pos();
  const Vector badguyMov = badguy->get_movement();

  // new vertical speed to test with
  float vy = -2*fabsf(badguyMov.x);

  // do not dive if we are not above the player
  float height = playerPos.y - badguyPos.y;
  if (height <= 0) return false;

  // do not dive if we would not descend faster than the player
  float relSpeed = -vy + playerMov.y;
  if (relSpeed <= 0) return false;

  // guess number of frames to descend to same height as player
  float estFrames = height / relSpeed;
  
  // guess where the player would be at this time
  float estPx = (playerPos.x + (estFrames * playerMov.x));

  // guess where we would be at this time
  float estBx = (badguyPos.x + (estFrames * badguyMov.x));

  // near misses are OK, too
  if (fabsf(estPx - estBx) < 32) return true;

  return false;
}

void 
Zeekling::active_update(float elapsed_time) {
  BadGuy::active_update(elapsed_time);

  if (state == FLYING) {
    if (should_we_dive()) {
      state = DIVING;
      physic.set_velocity_y(-2*fabsf(physic.get_velocity_x()));
      sprite->set_action(dir == LEFT ? "diving-left" : "diving-right");
    }
    return;
  }

  if (state == DIVING) {
    return;
  }

  if (state == CLIMBING) {
    // stop climbing when we're back at initial height
    if (get_pos().y <= start_position.y) {
      state = FLYING;
      physic.set_velocity_y(0);
    }
    return;
  }

}

IMPLEMENT_FACTORY(Zeekling, "zeekling")
