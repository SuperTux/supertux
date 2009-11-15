//  AngryStone - A spiked block that charges towards the player
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

#include "badguy/angrystone.hpp"

#include "lisp/writer.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

static const float SPEED = 240;

static const float CHARGE_TIME = .5;
static const float ATTACK_TIME = 1;
static const float RECOVER_TIME = .5;

AngryStone::AngryStone(const lisp::Lisp& reader) :
  BadGuy(reader, "images/creatures/angrystone/angrystone.sprite"), 
  attackDirection(),
  oldWallDirection(),
  timer(),
  state(IDLE)
{
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  physic.enable_gravity(true);
  sprite->set_action("idle");
}

void
AngryStone::write(lisp::Writer& writer)
{
  writer.start_list("angrystone");

  writer.write("x", start_position.x);
  writer.write("y", start_position.y);

  writer.end_list("angrystone");
}

void
AngryStone::collision_solid(const CollisionHit& hit)
{
  // TODO
  (void) hit;
#if 0
  if ((state == ATTACKING) &&
      (hit.normal.x == -attackDirection.x) && (hit.normal.y == attackDirection.y)) {
    state = IDLE;
    sprite->set_action("idle");
    physic.set_velocity_x(0);
    physic.set_velocity_y(0);
    physic.enable_gravity(true);
    oldWallDirection.x = attackDirection.x;
    oldWallDirection.y = attackDirection.y;
  }
#endif
}

void
AngryStone::kill_fall()
{
  //prevents AngryStone from getting killed by other enemies or the player
}

HitResponse
AngryStone::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  if (state == ATTACKING) {
    badguy.kill_fall();
    return FORCE_MOVE;
  }

  return FORCE_MOVE;
}

void
AngryStone::active_update(float elapsed_time) {
  BadGuy::active_update(elapsed_time);

  if (state == IDLE) {
    MovingObject* player = this->get_nearest_player();
    if(player) {
      MovingObject* badguy = this;
      const Vector playerPos = player->get_pos();
      const Vector badguyPos = badguy->get_pos();
      float dx = (playerPos.x - badguyPos.x);
      float dy = (playerPos.y - badguyPos.y);

      float playerHeight = player->get_bbox().p2.y - player->get_bbox().p1.y;
      float badguyHeight = badguy->get_bbox().p2.y - badguy->get_bbox().p1.y;

      float playerWidth = player->get_bbox().p2.x - player->get_bbox().p1.x;
      float badguyWidth = badguy->get_bbox().p2.x - badguy->get_bbox().p1.x;

      if ((dx > -playerWidth) && (dx < badguyWidth)) {
        if (dy > 0) {
          attackDirection.x = 0;
          attackDirection.y = 1;
        } else {
          attackDirection.x = 0;
          attackDirection.y = -1;
        }
        if ((attackDirection.x != oldWallDirection.x) || (attackDirection.y != oldWallDirection.y)) {
          sprite->set_action("charging");
          timer.start(CHARGE_TIME);
          state = CHARGING;
        }
      } else
        if ((dy > -playerHeight) && (dy < badguyHeight)) {
          if (dx > 0) {
            attackDirection.x = 1;
            attackDirection.y = 0;
          } else {
            attackDirection.x = -1;
            attackDirection.y = 0;
          }
          if ((attackDirection.x != oldWallDirection.x) || (attackDirection.y != oldWallDirection.y)) {
            sprite->set_action("charging");
            timer.start(CHARGE_TIME);
            state = CHARGING;
          }
        }

    }
  }

  if (state == CHARGING) {
    if (timer.check()) {
      sprite->set_action("attacking");
      timer.start(ATTACK_TIME);
      state = ATTACKING;
      physic.enable_gravity(false);
      physic.set_velocity_x(SPEED * attackDirection.x);
      physic.set_velocity_y(SPEED * attackDirection.y);
      oldWallDirection.x = 0;
      oldWallDirection.y = 0;
    }
  }

  if (state == ATTACKING) {
    if (timer.check()) {
      timer.start(RECOVER_TIME);
      state = RECOVERING;
      sprite->set_action("idle");
      physic.enable_gravity(true);
      physic.set_velocity_x(0);
      physic.set_velocity_y(0);
    }
  }

  if (state == RECOVERING) {
    if (timer.check()) {
      state = IDLE;
      sprite->set_action("idle");
      physic.enable_gravity(true);
      physic.set_velocity_x(0);
      physic.set_velocity_y(0);
    }
  }

}

IMPLEMENT_FACTORY(AngryStone, "angrystone");

/* EOF */
