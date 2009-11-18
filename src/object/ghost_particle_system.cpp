//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "object/ghost_particle_system.hpp"

#include <math.h>

#include "math/random_generator.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"

//FIXME: Sometimes both ghosts have the same image
//       Ghosts don't change their movement pattern - not random
GhostParticleSystem::GhostParticleSystem()
{
  ghosts[0] = new Surface("images/objects/particles/ghost0.png");
  ghosts[1] = new Surface("images/objects/particles/ghost1.png");

  virtual_width = SCREEN_WIDTH * 2;

  // create two ghosts
  size_t ghostcount = 2;
  for(size_t i=0; i<ghostcount; ++i) {
    GhostParticle* particle = new GhostParticle;
    particle->pos.x = systemRandom.randf(virtual_width);
    particle->pos.y = systemRandom.randf(SCREEN_HEIGHT);
    int size = systemRandom.rand(2);
    particle->texture = ghosts[size];
    particle->speed = systemRandom.randf(std::max(50, (size * 10)), 180 + (size * 10));
    particles.push_back(particle);
  }
}

void
GhostParticleSystem::parse(const Reader& reader)
{
  reader.get("z-pos", z_pos);
}

GhostParticleSystem::~GhostParticleSystem()
{
  for(int i=0;i<2;++i)
    delete ghosts[i];
}

void GhostParticleSystem::update(float elapsed_time)
{
  std::vector<Particle*>::iterator i;
  for(i = particles.begin(); i != particles.end(); ++i) {
    GhostParticle* particle = (GhostParticle*) *i;
    particle->pos.y -= particle->speed * elapsed_time;
    particle->pos.x -= particle->speed * elapsed_time;
    if(particle->pos.y > SCREEN_HEIGHT) {
      particle->pos.y = fmodf(particle->pos.y , virtual_height);
      particle->pos.x = systemRandom.rand(static_cast<int>(virtual_width));
    }
  }
}

/* EOF */
