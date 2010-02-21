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

#include "object/comet_particle_system.hpp"

#include "math/random_generator.hpp"
#include "supertux/globals.hpp"
#include "video/surface.hpp"
#include "util/reader.hpp"

CometParticleSystem::CometParticleSystem()
{
  cometimages[0] = Surface::create("images/creatures/mr_bomb/exploding-left-0.png");
  cometimages[1] = Surface::create("images/creatures/mr_bomb/exploding-left-0.png");

  virtual_width = SCREEN_WIDTH * 2;

  // create some random comets
  size_t cometcount = 2;
  for(size_t i=0; i<cometcount; ++i) {
    CometParticle* particle = new CometParticle;
    particle->pos.x = graphicsRandom.rand(int(virtual_width));
    particle->pos.y = graphicsRandom.rand(int(virtual_height));
    int cometsize = graphicsRandom.rand(2);
    particle->texture = cometimages[cometsize];
    do {
      particle->speed = (cometsize+1)*30 + graphicsRandom.randf(3.6);
    } while(particle->speed < 1);
    particle->speed *= 10; // gravity

    particles.push_back(particle);
  }
}

void
CometParticleSystem::parse(const Reader& reader)
{
  z_pos = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND1);
}

CometParticleSystem::~CometParticleSystem()
{
}

void CometParticleSystem::update(float elapsed_time)
{
  (void) elapsed_time;
#if 0
  std::vector<Particle*>::iterator i;
  for(
    i = particles.begin(); i != particles.end(); ++i) {
    CometParticle* particle = (CometParticle*) *i;
    float movement = particle->speed * elapsed_time;
    float abs_x = Sector::current()->camera->get_translation().x;
    float abs_y = Sector::current()->camera->get_translation().y;
    particle->pos.y += movement;
    particle->pos.x -= movement;
    int col = collision(particle, Vector(-movement, movement));
    if ((particle->pos.y > SCREEN_HEIGHT + abs_y) || (col >= 0)) {
      if ((particle->pos.y <= SCREEN_HEIGHT + abs_y) && (col >= 1)) {
        Sector::current()->add_object(new Bomb(particle->pos, LEFT));
      }
      int new_x = graphicsRandom.rand(int(virtual_width)) + int(abs_x);
      int new_y = 0;
      //FIXME: Don't move particles over solid tiles
      particle->pos.x = new_x;
      particle->pos.y = new_y;
    }
  }
#endif
}

/* EOF */
