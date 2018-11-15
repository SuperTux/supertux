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

#include "math/random.hpp"
#include "video/surface.hpp"

CloudParticleSystem::CloudParticleSystem() :
  ParticleSystem(128),
  cloudimage(Surface::from_file("images/objects/particles/cloud.png"))
{
  init();
}

CloudParticleSystem::CloudParticleSystem(const ReaderMapping& reader) :
  ParticleSystem(reader, 128),
  cloudimage(Surface::from_file("images/objects/particles/cloud.png"))
{
  init();
}

CloudParticleSystem::~CloudParticleSystem()
{
}

void CloudParticleSystem::init()
{
  virtual_width = 2000.0;

  // create some random clouds
  for (size_t i=0; i<15; ++i) {
    auto particle = std::make_unique<CloudParticle>();
    particle->pos.x = graphicsRandom.randf(virtual_width);
    particle->pos.y = graphicsRandom.randf(virtual_height);
    particle->texture = cloudimage;
    particle->speed = -graphicsRandom.randf(25.0, 54.0);

    particles.push_back(std::move(particle));
  }
}

void CloudParticleSystem::update(float dt_sec)
{
  if (!enabled)
    return;

  for (auto& particle : particles) {
    auto cloudParticle = dynamic_cast<CloudParticle*>(particle.get());
    if (!cloudParticle)
      continue;
    cloudParticle->pos.x += cloudParticle->speed * dt_sec;
  }
}

/* EOF */
