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
#include "random_generator.hpp"

Zeekling::Zeekling(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"), last_player(0)
{
  state = FLYING;
}

Zeekling::Zeekling(const Vector& pos, Direction d)
	: BadGuy(pos, d, "images/creatures/zeekling/zeekling.sprite"), last_player(0)
{
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
  speed = systemRandom.rand(130, 171);
  physic.vx = (dir == LEFT ? -speed : speed);
  physic.gravity_enabled = false;
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
Zeekling::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  kill_fall();
  return true;
}

void
Zeekling::onBumpHorizontal() {
  if (state == FLYING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.vx = (dir == LEFT ? -speed : speed);
  } else
  if (state == DIVING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    state = FLYING;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.vx = (dir == LEFT ? -speed : speed);
    physic.vy = 0;
  } else
  if (state == CLIMBING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.vx = (dir == LEFT ? -speed : speed);
  } else {
    assert(false);
  }
}

void
Zeekling::onBumpVertical() {
  if (state == FLYING) {
    physic.vy = 0;
  } else
  if (state == DIVING) {
    state = CLIMBING;
    physic.vy = -speed;
    sprite->set_action(dir == LEFT ? "left" : "right");
  } else
  if (state == CLIMBING) {
    state = FLYING;
    physic.vy = 0;
  }
}

void
Zeekling::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    onBumpVertical();
  } else if(hit.left || hit.right) {
    onBumpHorizontal();
  }
}

/**
 * linear prediction of player and badguy positions to decide if we should enter the DIVING state
 */
bool
Zeekling::should_we_dive() {

  const MovingObject* player = this->get_nearest_player();
  if (player && last_player && (player == last_player)) {

    // get positions, calculate movement
    const Vector player_pos = player->get_pos();
    const Vector player_mov = (player_pos - last_player_pos);
    const Vector self_pos = this->get_pos();
    const Vector self_mov = (self_pos - last_self_pos);

    // new vertical speed to test with
    float vy = 2*fabsf(self_mov.x);

    // do not dive if we are not above the player
    float height = player_pos.y - self_pos.y;
    if (height <= 0) return false;

    // do not dive if we are too far above the player
    if (height > 512) return false;

    // do not dive if we would not descend faster than the player
    float relSpeed = vy - player_mov.y;
    if (relSpeed <= 0) return false;

    // guess number of frames to descend to same height as player
    float estFrames = height / relSpeed;

    // guess where the player would be at this time
    float estPx = (player_pos.x + (estFrames * player_mov.x));

    // guess where we would be at this time
    float estBx = (self_pos.x + (estFrames * self_mov.x));

    // near misses are OK, too
    if (fabsf(estPx - estBx) < 8) return true;
  }

  // update last player tracked, as well as our positions
  last_player = player;
  if (player) {
    last_player_pos = player->get_pos();
    last_self_pos = this->get_pos();
  }

  return false;
}

void
Zeekling::active_update(float elapsed_time) {
  if (state == FLYING) {
    if (should_we_dive()) {
      state = DIVING;
      physic.vy = 2*fabsf(physic.vx);
      sprite->set_action(dir == LEFT ? "diving-left" : "diving-right");
    }
    BadGuy::active_update(elapsed_time);
    return;
  } else if (state == DIVING) {
    BadGuy::active_update(elapsed_time);
    return;
  } else if (state == CLIMBING) {
    // stop climbing when we're back at initial height
    if (get_pos().y <= start_position.y) {
      state = FLYING;
      physic.vy = 0;
    }
    BadGuy::active_update(elapsed_time);
    return;
  } else {
    assert(false);
  }
}

IMPLEMENT_FACTORY(Zeekling, "zeekling")
