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

#include "badguy/flyingsnowball.hpp"

#include "math/random_generator.hpp"
#include "object/sprite_particle.hpp"
#include "object/player.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

namespace {
const float PUFF_INTERVAL_MIN = 4.0f; /**< spawn new puff of smoke at most that often */
const float PUFF_INTERVAL_MAX = 8.0f; /**< spawn new puff of smoke at least that often */
}

FlyingSnowBall::FlyingSnowBall(const Reader& reader) :
  BadGuy(reader, "images/creatures/flying_snowball/flying_snowball.sprite"),
  normal_propeller_speed(),
  puff_timer()
{
  physic.enable_gravity(true);
}

FlyingSnowBall::FlyingSnowBall(const Vector& pos) :
  BadGuy(pos, "images/creatures/flying_snowball/flying_snowball.sprite"),
  normal_propeller_speed(),
  puff_timer()
{
  physic.enable_gravity(true);
}

void
FlyingSnowBall::initialize()
{
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
FlyingSnowBall::activate()
{
  puff_timer.start(gameRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX));
  normal_propeller_speed = gameRandom.randf(0.95, 1.05);
}

bool
FlyingSnowBall::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  physic.set_acceleration_y(0);
  physic.set_velocity_y(0);
  kill_squished(object);
  return true;
}

void
FlyingSnowBall::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  }
}

void
FlyingSnowBall::active_update(float elapsed_time)
{

  const float grav = Sector::current()->get_gravity() * 100.0f;
  if (get_pos().y > start_position.y + 2*32) {

    // Flying too low - increased propeller speed
    physic.set_acceleration_y(-grav*1.2);

    physic.set_velocity_y(physic.get_velocity_y() * 0.99);

  } else if (get_pos().y < start_position.y - 2*32) {

    // Flying too high - decreased propeller speed
    physic.set_acceleration_y(-grav*0.8);

    physic.set_velocity_y(physic.get_velocity_y() * 0.99f);

  } else {

    // Flying at acceptable altitude - normal propeller speed
    physic.set_acceleration_y(-grav*normal_propeller_speed);

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
    Vector pspeed = Vector(gameRandom.randf(-10, 10), 150);
    Vector paccel = Vector(0,0);
    Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/smoke.sprite",
                                                                   "default",
                                                                   ppos, ANCHOR_MIDDLE, pspeed, paccel,
                                                                   LAYER_OBJECTS-1));
    puff_timer.start(gameRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX));

    normal_propeller_speed = gameRandom.randf(0.95, 1.05);
    physic.set_velocity_y(physic.get_velocity_y() - 50);
  }
}

/* EOF */
