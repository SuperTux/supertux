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
#include "supertux/main.hpp"
#include "video/drawing_context.hpp"

SnowParticleSystem::SnowParticleSystem()
{
  snowimages[0] = new Surface("images/objects/particles/snow2.png");
  snowimages[1] = new Surface("images/objects/particles/snow1.png");
  snowimages[2] = new Surface("images/objects/particles/snow0.png");

  virtual_width = SCREEN_WIDTH * 2;

  // create some random snowflakes
  size_t snowflakecount = size_t(virtual_width/10.0);
  for(size_t i=0; i<snowflakecount; ++i) {
    SnowParticle* particle = new SnowParticle;
    int snowsize = systemRandom.rand(3);

    particle->pos.x = systemRandom.randf(virtual_width);
    particle->pos.y = systemRandom.randf(SCREEN_HEIGHT);
    particle->anchorx = particle->pos.x + (systemRandom.randf(-0.5, 0.5) * 16);
    particle->drift_speed = systemRandom.randf(-0.5, 0.5) * 0.3;
    particle->wobble = 0.0;

    particle->texture = snowimages[snowsize];

    particle->speed = 1 + (2 - snowsize)/2 + systemRandom.randf(1.8);
    particle->speed *= 20; // gravity

    particles.push_back(particle);
  }
}

void
SnowParticleSystem::parse(const Reader& reader)
{
  reader.get("z-pos", z_pos);
}

SnowParticleSystem::~SnowParticleSystem()
{
  for(int i=0;i<3;++i)
    delete snowimages[i];
}

void SnowParticleSystem::update(float elapsed_time)
{
  std::vector<Particle*>::iterator i;

  for(i = particles.begin(); i != particles.end(); ++i) {
    SnowParticle* particle = (SnowParticle*) *i;
    float anchor_delta;

    particle->pos.y += particle->speed * elapsed_time;
    particle->pos.x += particle->wobble * elapsed_time /* * particle->speed * 0.125*/;

    anchor_delta = (particle->anchorx - particle->pos.x);
    particle->wobble += (4 * anchor_delta * 0.05) + systemRandom.randf(-0.5, 0.5);
    particle->wobble *= 0.99f;
    particle->anchorx += particle->drift_speed * elapsed_time;
  }
}

/* EOF */
