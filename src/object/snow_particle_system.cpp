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

#include "object/snow_particle_system.hpp"

#include <assert.h>
#include <math.h>

#include "math/random.hpp"
#include "supertux/sector.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

// TODO: tweak values
namespace SNOW {
static const float SPIN_SPEED = 60.0f;
static const float WIND_SPEED = 30.0f; // max speed of wind will be randf(WIND_SPEED) * randf(STATE_LENGTH)
static const float STATE_LENGTH = 5.0f;
static const float DECAY_RATIO = 0.2f; // ratio of attack speed to decay speed
static const float EPSILON = 0.5f; //velocity changes by up to this much each tick
static const float WOBBLE_DECAY = 0.99f; //wobble decays exponentially by this much each tick
static const float WOBBLE_FACTOR = 4 * .005f; //wobble approaches drift_speed by this much each tick
}

SnowParticleSystem::SnowParticleSystem() :
  state(RELEASING),
  timer(),
  gust_onset(0),
  gust_current_velocity(0)
{
  init();
}

SnowParticleSystem::SnowParticleSystem(const ReaderMapping& reader) :
  ParticleSystem(reader),
  state(RELEASING),
  timer(),
  gust_onset(0),
  gust_current_velocity(0)
{
  init();
}

SnowParticleSystem::~SnowParticleSystem()
{
}

void SnowParticleSystem::init()
{
  snowimages[0] = Surface::from_file("images/particles/snow2.png");
  snowimages[1] = Surface::from_file("images/particles/snow1.png");
  snowimages[2] = Surface::from_file("images/particles/snow0.png");

  virtual_width = static_cast<float>(SCREEN_WIDTH) * 2.0f;

  timer.start(.01f);

  // create some random snowflakes
  int snowflakecount = static_cast<int>(virtual_width / 10.0f);
  for (int i = 0; i < snowflakecount; ++i) {
    auto particle = std::make_unique<SnowParticle>();
    int snowsize = graphicsRandom.rand(3);

    particle->pos.x = graphicsRandom.randf(virtual_width);
    particle->pos.y = graphicsRandom.randf(static_cast<float>(SCREEN_HEIGHT));
    particle->anchorx = particle->pos.x + (graphicsRandom.randf(-0.5, 0.5) * 16);
    // drift will change with wind gusts
    particle->drift_speed = graphicsRandom.randf(-0.5f, 0.5f) * 0.3f;
    particle->wobble = 0.0;

    particle->texture = snowimages[snowsize];
    particle->flake_size = static_cast<int>(powf(static_cast<float>(snowsize) + 3.0f, 4.0f)); // since it ranges from 0 to 2

    particle->speed = 6.32f * (1.0f + (2.0f - static_cast<float>(snowsize)) / 2.0f + graphicsRandom.randf(1.8f));

    // Spinning
    particle->angle = graphicsRandom.randf(360.0);
    particle->spin_speed = graphicsRandom.randf(-SNOW::SPIN_SPEED,SNOW::SPIN_SPEED);

    particles.push_back(std::move(particle));
  }
}

void SnowParticleSystem::update(float dt_sec)
{
  if (!enabled)
    return;

  // Simple ADSR wind gusts

  if (timer.check()) {
    // Change state
    state = static_cast<State>((state + 1) % MAX_STATE);

    if (state == RESTING) {
      // stop wind
      gust_current_velocity = 0;
      // new wind strength
      gust_onset = graphicsRandom.randf(-SNOW::WIND_SPEED, SNOW::WIND_SPEED);
    }
    timer.start(graphicsRandom.randf(SNOW::STATE_LENGTH));
  }

  // Update velocities
  switch (state) {
    case ATTACKING:
      gust_current_velocity += gust_onset * dt_sec;
      break;
    case DECAYING:
      gust_current_velocity -= gust_onset * dt_sec * SNOW::DECAY_RATIO;
      break;
    case RELEASING:
      // uses current time/velocity instead of constants
      gust_current_velocity -= gust_current_velocity * dt_sec / timer.get_timeleft();
      break;
    case SUSTAINING:
    case RESTING:
      //do nothing
      break;
    default:
      assert(false);
  }

  float sq_g = sqrtf(Sector::get().get_gravity());

  for (auto& part : particles) {
    auto particle = dynamic_cast<SnowParticle*>(part.get());
    if (!particle)
      continue;

    float anchor_delta;

    // Falling
    particle->pos.y += particle->speed * dt_sec * sq_g;
    // Drifting (speed approaches wind at a rate dependent on flake size)
    particle->drift_speed += (gust_current_velocity - particle->drift_speed) / static_cast<float>(particle->flake_size) + graphicsRandom.randf(-SNOW::EPSILON, SNOW::EPSILON);
    particle->anchorx += particle->drift_speed * dt_sec;
    // Wobbling (particle approaches anchorx)
    particle->pos.x += particle->wobble * dt_sec * sq_g;
    anchor_delta = (particle->anchorx - particle->pos.x);
    particle->wobble += (SNOW::WOBBLE_FACTOR * anchor_delta) + graphicsRandom.randf(-SNOW::EPSILON, SNOW::EPSILON);
    particle->wobble *= SNOW::WOBBLE_DECAY;
    // Spinning
    particle->angle += particle->spin_speed * dt_sec;
    particle->angle = fmodf(particle->angle, 360.0);
  }
}

/* EOF */
