//  Toad - A jumping toad
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
#include <config.h>

#include "badguy/toad.hpp"

#include "math/random_generator.hpp"
#include "lisp/writer.hpp"
#include "supertux/object_factory.hpp"
#include "object/player.hpp"
#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"

namespace {
  const float VERTICAL_SPEED = -450;   /**< y-speed when jumping */
  const float HORIZONTAL_SPEED = 320; /**< x-speed when jumping */
  const float RECOVER_TIME = 0.5; /**< time to stand still before starting a (new) jump */
  static const std::string HOP_SOUND = "sounds/hop.ogg";
}

Toad::Toad(const lisp::Lisp& reader)
        : BadGuy(reader, "images/creatures/toad/toad.sprite")
{
  sound_manager->preload(HOP_SOUND);
}

Toad::Toad(const Vector& pos, Direction d)
        : BadGuy(pos, d, "images/creatures/toad/toad.sprite")
{
  sound_manager->preload(HOP_SOUND);
}

void
Toad::write(lisp::Writer& writer)
{
  writer.start_list("toad");
  writer.write("x", start_position.x);
  writer.write("y", start_position.y);
  writer.end_list("toad");
}

void
Toad::initialize()
{
  // initial state is JUMPING, because we might start airborne
  state = JUMPING;
  sprite->set_action(dir == LEFT ? "jumping-left" : "jumping-right");
}

void
Toad::set_state(ToadState newState)
{
  if (newState == IDLE) {
    physic.set_velocity_x(0);
    physic.set_velocity_y(0);
    sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");

    recover_timer.start(RECOVER_TIME);
  } else
  if (newState == JUMPING) {
    sprite->set_action(dir == LEFT ? "jumping-left" : "jumping-right");
    physic.set_velocity_x(dir == LEFT ? -HORIZONTAL_SPEED : HORIZONTAL_SPEED);
    physic.set_velocity_y(VERTICAL_SPEED);
    sound_manager->play( HOP_SOUND, get_pos());
  } else
  if (newState == FALLING) {
    Player* player = get_nearest_player();
    // face player
    if (player && (player->get_bbox().p2.x < get_bbox().p1.x) && (dir == RIGHT)) dir = LEFT;
    if (player && (player->get_bbox().p1.x > get_bbox().p2.x) && (dir == LEFT)) dir = RIGHT;
    sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");
  }

  state = newState;
}

bool
Toad::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

void
Toad::collision_solid(const CollisionHit& hit)
{
  // just default behaviour (i.e. stop at floor/walls) when squished
  if (BadGuy::get_state() == STATE_SQUISHED) {
    BadGuy::collision_solid(hit);
    return;
  }

  // ignore collisions while standing still
  if(state == IDLE) {
    return;
  }

  // check if we hit left or right while moving in either direction
  if(((physic.get_velocity_x() < 0) && hit.left) || ((physic.get_velocity_x() > 0) && hit.right)) {
    /*
    dir = dir == LEFT ? RIGHT : LEFT;
    if (state == JUMPING) {
      sprite->set_action(dir == LEFT ? "jumping-left" : "jumping-right");
    } else {
      sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");
    }
    */
    physic.set_velocity_x(-0.25*physic.get_velocity_x());
  }

  // check if we hit the floor while falling
  if ((state == FALLING) && hit.bottom) {
    set_state(IDLE);
    return;
  }

  // check if we hit the roof while climbing
  if ((state == JUMPING) && hit.top) {
    physic.set_velocity_y(0);
  }

}

HitResponse
Toad::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  // behaviour for badguy collisions is the same as for collisions with solids
  collision_solid(hit);

  return CONTINUE;
}

void
Toad::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  // change sprite when we are falling
  if ((state == JUMPING) && (physic.get_velocity_y() > 0)) {
    set_state(FALLING);
    return;
  }

  // jump when fully recovered
  if ((state == IDLE) && (recover_timer.check())) {
    set_state(JUMPING);
    return;
  }

}

IMPLEMENT_FACTORY(Toad, "toad");
