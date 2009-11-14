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

#include <config.h>

#include "stalactite.hpp"

#include "math/random_generator.hpp"
#include "lisp/writer.hpp"
#include "supertux/object_factory.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"

static const int SHAKE_RANGE_X = 40;
static const float SHAKE_TIME = .8f;
static const float SQUISH_TIME = 2;
static const float SHAKE_RANGE_Y = 400;

Stalactite::Stalactite(const lisp::Lisp& lisp)
        : BadGuy(lisp, "images/creatures/stalactite/stalactite.sprite", LAYER_TILES - 1), state(STALACTITE_HANGING)
{
  countMe = false;
  set_colgroup_active(COLGROUP_TOUCHABLE);
}

void
Stalactite::write(lisp::Writer& writer)
{
  writer.start_list("stalactite");
  writer.write("x", start_position.x);
  writer.write("y", start_position.y);
  writer.end_list("stalactite");
}

void
Stalactite::active_update(float elapsed_time)
{
  if(state == STALACTITE_HANGING) {
    Player* player = this->get_nearest_player();
    if (player) {
      if(player->get_bbox().p2.x > bbox.p1.x - SHAKE_RANGE_X
          && player->get_bbox().p1.x < bbox.p2.x + SHAKE_RANGE_X
          && player->get_bbox().p2.y > bbox.p1.y
          && player->get_bbox().p1.y < bbox.p2.y + SHAKE_RANGE_Y) {
        timer.start(SHAKE_TIME);
        state = STALACTITE_SHAKING;
      }
    }
  } else if(state == STALACTITE_SHAKING) {
    if(timer.check()) {
      state = STALACTITE_FALLING;
      physic.enable_gravity(true);
      set_colgroup_active(COLGROUP_MOVING);
    }
  } else if(state == STALACTITE_FALLING || state == STALACTITE_SQUISHED) {
    movement = physic.get_movement(elapsed_time);
    if(state == STALACTITE_SQUISHED && timer.check())
      remove_me();
  }
}

void
Stalactite::squish()
{
  state = STALACTITE_SQUISHED;
  set_colgroup_active(COLGROUP_MOVING_ONLY_STATIC);
  sprite->set_action("squished");
  if(!timer.started())
    timer.start(SQUISH_TIME);
}

void
Stalactite::collision_solid(const CollisionHit& hit)
{
  if(state == STALACTITE_FALLING) {
    if (hit.bottom) squish();
  }
  if(state == STALACTITE_SQUISHED) {
    physic.set_velocity_y(0);
  }
}

HitResponse
Stalactite::collision_player(Player& player)
{
  if(state != STALACTITE_SQUISHED) {
    player.kill(false);
  }

  return FORCE_MOVE;
}

HitResponse
Stalactite::collision_badguy(BadGuy& other, const CollisionHit& hit)
{
  if (state == STALACTITE_SQUISHED) return FORCE_MOVE;

  // ignore other Stalactites
  if (dynamic_cast<Stalactite*>(&other)) return FORCE_MOVE;

  if (state != STALACTITE_FALLING) return BadGuy::collision_badguy(other, hit);

  if (other.is_freezable()) {
    other.freeze();
  } else {
    other.kill_fall();
  }

  remove_me();

  return FORCE_MOVE;
}

void
Stalactite::kill_fall()
{
}

void
Stalactite::draw(DrawingContext& context)
{
  if(get_state() != STATE_ACTIVE)
    return;


  if(state == STALACTITE_SQUISHED) {
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
    return;
  }

  if(state == STALACTITE_SHAKING) {
    sprite->draw(context, get_pos() + Vector(systemRandom.rand(-3,3), 0), layer);
  } else {
    sprite->draw(context, get_pos(), layer);
  }
}

void
Stalactite::deactivate()
{
  if(state != STALACTITE_HANGING)
    remove_me();
}

IMPLEMENT_FACTORY(Stalactite, "stalactite");
