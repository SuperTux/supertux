//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include <algorithm>
#include <math.h>

#include "math/random_generator.hpp"
#include "supertux/globals.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"

//FIXME: Sometimes both ghosts have the same image
//       Ghosts don't change their movement pattern - not random
GhostParticleSystem::GhostParticleSystem()
{
  ghosts[0] = Surface::create("images/objects/particles/ghost0.png");
  ghosts[1] = Surface::create("images/objects/particles/ghost1.png");

  virtual_width = SCREEN_WIDTH * 2;

  // create two ghosts
  size_t ghostcount = 2;
  for(size_t i=0; i<ghostcount; ++i) {
    auto particle = std::unique_ptr<GhostParticle>(new GhostParticle);
    particle->pos.x = graphicsRandom.randf(virtual_width);
    particle->pos.y = graphicsRandom.randf(SCREEN_HEIGHT);
    int size = graphicsRandom.rand(2);
    particle->texture = ghosts[size];
    particle->speed = graphicsRandom.randf(std::max(50, (size * 10)), 180 + (size * 10));
    particles.push_back(std::move(particle));
  }
}

void
GhostParticleSystem::parse(const ReaderMapping& reader)
{
  ParticleSystem::parse(reader);
  z_pos = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND1);
}

GhostParticleSystem::~GhostParticleSystem()
{
}

void GhostParticleSystem::update(float elapsed_time)
{
  if(!enabled)
    return;

  for(auto i = particles.begin(); i != particles.end(); ++i) {
    GhostParticle* particle = (GhostParticle*)i->get();
    particle->pos.y -= particle->speed * elapsed_time;
    particle->pos.x -= particle->speed * elapsed_time;
    if(particle->pos.y > SCREEN_HEIGHT) {
      particle->pos.y = fmodf(particle->pos.y , virtual_height);
      particle->pos.x = graphicsRandom.rand(static_cast<int>(virtual_width));
    }
  }
}

/* EOF */
