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

#include <math.h>

#include "math/random_generator.hpp"
#include "supertux/globals.hpp"
#include "util/gettext.hpp"
#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"

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
  snowimages[0] = Surface::create("images/objects/particles/snow2.png");
  snowimages[1] = Surface::create("images/objects/particles/snow1.png");
  snowimages[2] = Surface::create("images/objects/particles/snow0.png");

  virtual_width = SCREEN_WIDTH * 2;

  timer.start(.01);

  // create some random snowflakes
  size_t snowflakecount = size_t(virtual_width/10.0);
  for(size_t i=0; i<snowflakecount; ++i) {
    SnowParticle* particle = new SnowParticle;
    int snowsize = graphicsRandom.rand(3);

    particle->pos.x = graphicsRandom.randf(virtual_width);
    particle->pos.y = graphicsRandom.randf(SCREEN_HEIGHT);
    particle->anchorx = particle->pos.x + (graphicsRandom.randf(-0.5, 0.5) * 16);
    // drift will change with wind gusts
    particle->drift_speed = graphicsRandom.randf(-0.5, 0.5) * 0.3;
    particle->wobble = 0.0;

    particle->texture = snowimages[snowsize];
    particle->flake_size = powf(snowsize+3,4); // since it ranges from 0 to 2

    particle->speed = 2 * (1 + (2 - snowsize)/2 + graphicsRandom.randf(1.8)) * 10; // gravity

    // Spinning
    particle->angle = graphicsRandom.randf(360.0);
    particle->spin_speed = graphicsRandom.randf(-SNOW::SPIN_SPEED,SNOW::SPIN_SPEED);

    particles.push_back(particle);
  }
}

void
SnowParticleSystem::parse(const Reader& reader)
{
  z_pos = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND1);
}

SnowParticleSystem::~SnowParticleSystem()
{
}

void SnowParticleSystem::update(float elapsed_time)
{
  // Simple ADSR wind gusts

  if (timer.check()) {
    // Change state
    state = (State) ((state + 1) % MAX_STATE);

    if(state == RESTING) {
      // stop wind
      gust_current_velocity = 0;
      // new wind strength
      gust_onset   = graphicsRandom.randf(-SNOW::WIND_SPEED, SNOW::WIND_SPEED);
    }
    timer.start(graphicsRandom.randf(SNOW::STATE_LENGTH));
  }

  // Update velocities
  switch(state) {
    case ATTACKING:
      gust_current_velocity += gust_onset * elapsed_time;
      break;
    case DECAYING:
      gust_current_velocity -= gust_onset * elapsed_time * SNOW::DECAY_RATIO;
      break;
    case RELEASING:
      // uses current time/velocity instead of constants
      gust_current_velocity -= gust_current_velocity * elapsed_time / timer.get_timeleft();
      break;
    case SUSTAINING:
    case RESTING:
      //do nothing
      break;
    default:
      assert(false);
  }

  std::vector<Particle*>::iterator i;

  for(i = particles.begin(); i != particles.end(); ++i) {
    SnowParticle* particle = (SnowParticle*) *i;
    float anchor_delta;

    // Falling
    particle->pos.y += particle->speed * elapsed_time;
    // Drifting (speed approaches wind at a rate dependent on flake size)
    particle->drift_speed += (gust_current_velocity - particle->drift_speed) / particle->flake_size + graphicsRandom.randf(-SNOW::EPSILON,SNOW::EPSILON);
    particle->anchorx += particle->drift_speed * elapsed_time;
    // Wobbling (particle approaches anchorx)
    particle->pos.x += particle->wobble * elapsed_time;
    anchor_delta = (particle->anchorx - particle->pos.x);
    particle->wobble += (SNOW::WOBBLE_FACTOR * anchor_delta) + graphicsRandom.randf(-SNOW::EPSILON, SNOW::EPSILON);
    particle->wobble *= SNOW::WOBBLE_DECAY;
    // Spinning
    particle->angle += particle->spin_speed * elapsed_time;
    particle->angle = fmodf(particle->angle, 360.0);
  }
}

ObjectSettings
SnowParticleSystem::get_settings() {
  ObjectSettings result(_("Snow particles"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &z_pos));

  return result;
}

/* EOF */
