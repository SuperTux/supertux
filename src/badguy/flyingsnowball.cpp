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

#include "math/random.hpp"
#include "object/sprite_particle.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

namespace {
const float PUFF_INTERVAL_MIN = 4.0f; /**< spawn new puff of smoke at most that often */
const float PUFF_INTERVAL_MAX = 8.0f; /**< spawn new puff of smoke at least that often */
}

FlyingSnowBall::FlyingSnowBall(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/flying_snowball/flying_snowball.sprite"),
  total_time_elapsed(),
  puff_timer()
{
  m_physic.enable_gravity(true);
}

void
FlyingSnowBall::initialize()
{
  m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
}

void
FlyingSnowBall::activate()
{
  puff_timer.start(static_cast<float>(gameRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX)));
}

bool
FlyingSnowBall::collision_squished(GameObject& object)
{
  m_sprite->set_action(m_dir == Direction::LEFT ? "squished-left" : "squished-right");
  m_physic.set_acceleration_y(0);
  m_physic.set_velocity_y(0);
  kill_squished(object);
  return true;
}

void
FlyingSnowBall::collision_solid(const CollisionHit& hit)
{
  if (hit.top || hit.bottom) {
    m_physic.set_velocity_y(0);
  }
}

void
FlyingSnowBall::active_update(float dt_sec)
{
  total_time_elapsed += dt_sec;
  
  const float grav = Sector::get().get_gravity() * 100.0f;
  if (get_pos().y > m_start_position.y + 2*32) {

    // Flying too low - increased propeller speed
    m_physic.set_acceleration_y(-grav*1.2f);

    m_physic.set_velocity_y(m_physic.get_velocity_y() * 0.99f);

  } else if (get_pos().y < m_start_position.y - 2*32) {

    // Flying too high - decreased propeller speed
    m_physic.set_acceleration_y(-grav*0.8f);

    m_physic.set_velocity_y(m_physic.get_velocity_y() * 0.99f);

  } else {

    // Flying at acceptable altitude - normal propeller speed

    float direction = std::pow(std::sin(total_time_elapsed/3.5f), 3.f) +
                      std::sin(3.f *
                               ((total_time_elapsed/2.5f - 3.14159f) / 3.f)
                              ) / 3.f;

    // If you have a graphing calculator, type this :
    //    sin(x)^3 + sin(3 * (x - pi / 3)) / 3
    // It seems like a good regular pattern to me, however I didn't implement
    // it properly here as I didn't want to change the pattern too much (it
    // should still *look* random unless otherwise recommended.
    //
    // I believe a badguy's pattern should be easy to understand and predict,
    // but I haven't discussed about it yet with the rest of the team.
    //    ~ Semphris

    m_physic.set_acceleration_y(-grav * (1.f + direction / 13.f));

  }

  m_col.m_movement=m_physic.get_movement(dt_sec);

  auto player = get_nearest_player();
  if (player) {
    m_dir = (player->get_pos().x > get_pos().x) ? Direction::RIGHT : Direction::LEFT;
    m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
  }

  // spawn smoke puffs
  if (puff_timer.check()) {
    Vector ppos = m_col.m_bbox.get_middle();
    Vector pspeed = Vector(gameRandom.randf(-10, 10), 150);
    Vector paccel = Vector(0,0);
    Sector::get().add<SpriteParticle>("images/particles/smoke.sprite",
                                           "default",
                                           ppos, ANCHOR_MIDDLE, pspeed, paccel,
                                           LAYER_OBJECTS-1);
    puff_timer.start(gameRandom.randf(PUFF_INTERVAL_MIN, PUFF_INTERVAL_MAX));

    //m_physic.set_velocity_y(m_physic.get_velocity_y() - 50);
  }
  
}

/* EOF */
