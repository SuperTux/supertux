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

#include <stdio.h>

#include "flyingsnowball.hpp"
#include "random_generator.hpp"
#include "object/sprite_particle.hpp"

static const float FLYTIME = 1.0;
static const float FLYSPEED = -100.0;

namespace {
  const float PUFF_PROBABILITY = 0.1; /**< chanche of puffs being spawned in the current cycle */
  const float PUFF_INTERVAL_MIN = 0.1; /**< spawn new puff of smoke at most that often */
  const float PUFF_INTERVAL_MAX = 1.1; /**< spawn new puff of smoke at least that often */
}

FlyingSnowBall::FlyingSnowBall(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/flying_snowball/flying_snowball.sprite")
{
  physic.enable_gravity(false);
}

FlyingSnowBall::FlyingSnowBall(const Vector& pos)
	: BadGuy(pos, "images/creatures/flying_snowball/flying_snowball.sprite")
{
  physic.enable_gravity(false);
}

void
FlyingSnowBall::write(lisp::Writer& writer)
{
  writer.start_list("flyingsnowball");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("flyingsnowball");
}

void 
FlyingSnowBall::activate()
{
  sprite->set_action(dir == LEFT ? "left" : "right");
  mode = FLY_UP;
  physic.set_velocity_y(FLYSPEED);
  timer.start(FLYTIME/2);
  puff_timer.start(systemRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX));
}

bool
FlyingSnowBall::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  return true;
}

HitResponse
FlyingSnowBall::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
FlyingSnowBall::active_update(float elapsed_time) 
{
  if(timer.check()) {
    if(mode == FLY_UP) {
      mode = FLY_DOWN;
      physic.set_velocity_y(-FLYSPEED);

      // stop puffing
      puff_timer.stop();
      
    } else if(mode == FLY_DOWN) {
      mode = FLY_UP;
      physic.set_velocity_y(FLYSPEED);

      // roll a dice whether to start puffing
      if (systemRandom.randf(0, 1) < PUFF_PROBABILITY) {
        puff_timer.start(systemRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX));
      }

    }
    timer.start(FLYTIME);
  }
  movement=physic.get_movement(elapsed_time);

  Player* player = this->get_nearest_player();
  if (player) {
    dir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
  }

  // spawn smoke puffs
  if (puff_timer.check()) {
    Vector ppos = bbox.get_middle();
    Vector pspeed = Vector(systemRandom.randf(-10, 10), 150);
    Vector paccel = Vector(0,0);
    Sector::current()->add_object(new SpriteParticle("images/objects/particles/smoke.sprite", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1));
    puff_timer.start(systemRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX));
  }
}

IMPLEMENT_FACTORY(FlyingSnowBall, "flyingsnowball")
