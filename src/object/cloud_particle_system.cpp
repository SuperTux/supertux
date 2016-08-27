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

#include "object/cloud_particle_system.hpp"

#include <math.h>

#include "math/random_generator.hpp"
#include "supertux/globals.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

CloudParticleSystem::CloudParticleSystem() :
  ParticleSystem(128),
  cloudimage(Surface::create("images/objects/particles/cloud.png"))
{
  init();
}

CloudParticleSystem::CloudParticleSystem(const ReaderMapping& reader) :
  ParticleSystem(128),
  cloudimage(Surface::create("images/objects/particles/cloud.png"))
{
  init();
  parse(reader);
}

CloudParticleSystem::~CloudParticleSystem()
{
}

void CloudParticleSystem::init()
{
  virtual_width = 2000.0;

  // create some random clouds
  for(size_t i=0; i<15; ++i) {
    auto particle = std::unique_ptr<CloudParticle>(new CloudParticle);
    particle->pos.x = graphicsRandom.rand(static_cast<int>(virtual_width));
    particle->pos.y = graphicsRandom.rand(static_cast<int>(virtual_height));
    particle->texture = cloudimage;
    particle->speed = -graphicsRandom.randf(25.0, 54.0);

    particles.push_back(std::move(particle));
  }
}

void CloudParticleSystem::update(float elapsed_time)
{
  if(!enabled)
    return;

  for(auto& particle : particles) {
    auto cloudParticle = dynamic_cast<CloudParticle*>(particle.get());
    if (!cloudParticle)
      continue;
    cloudParticle->pos.x += cloudParticle->speed * elapsed_time;
  }
}

/* EOF */
