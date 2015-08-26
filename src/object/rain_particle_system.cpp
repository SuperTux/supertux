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

#include "object/rain_particle_system.hpp"

#include "math/random_generator.hpp"
#include "object/camera.hpp"
#include "object/rainsplash.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"

RainParticleSystem::RainParticleSystem()
{
  rainimages[0] = Surface::create("images/objects/particles/rain0.png");
  rainimages[1] = Surface::create("images/objects/particles/rain1.png");

  virtual_width = SCREEN_WIDTH * 2;

  // create some random raindrops
  size_t raindropcount = size_t(virtual_width/6.0);
  for(size_t i=0; i<raindropcount; ++i) {
    RainParticle* particle = new RainParticle;
    particle->pos.x = graphicsRandom.rand(int(virtual_width));
    particle->pos.y = graphicsRandom.rand(int(virtual_height));
    int rainsize = graphicsRandom.rand(2);
    particle->texture = rainimages[rainsize];
    do {
      particle->speed = (rainsize+1)*45 + graphicsRandom.randf(3.6);
    } while(particle->speed < 1);

    particles.push_back(particle);
  }
}

RainParticleSystem::~RainParticleSystem()
{
}

void RainParticleSystem::update(float elapsed_time)
{
  std::vector<Particle*>::iterator i;
  for(
    i = particles.begin(); i != particles.end(); ++i) {
    RainParticle* particle = (RainParticle*) *i;
    float movement = particle->speed * elapsed_time * Sector::current()->get_gravity();
    float abs_x = Sector::current()->camera->get_translation().x;
    float abs_y = Sector::current()->camera->get_translation().y;
    particle->pos.y += movement;
    particle->pos.x -= movement;
    int col = collision(particle, Vector(-movement, movement));
    if ((particle->pos.y > SCREEN_HEIGHT + abs_y) || (col >= 0)) {
      //Create rainsplash
      if ((particle->pos.y <= SCREEN_HEIGHT + abs_y) && (col >= 1)){
        bool vertical = (col == 2);
        if (!vertical) { //check if collision happened from above
          int splash_x, splash_y; // move outside if statement when
                                  // uncommenting the else statement below.
          splash_x = int(particle->pos.x);
          splash_y = int(particle->pos.y) - (int(particle->pos.y) % 32) + 32;
          Sector::current()->add_object(std::make_shared<RainSplash>(Vector(splash_x, splash_y),vertical));
        }
        // Uncomment the following to display vertical splashes, too
        /* else {
           splash_x = int(particle->pos.x) - (int(particle->pos.x) % 32) + 32;
           splash_y = int(particle->pos.y);
           Sector::current()->add_object(new RainSplash(Vector(splash_x, splash_y),vertical));
           } */
      }
      int new_x = graphicsRandom.rand(int(virtual_width)) + int(abs_x);
      int new_y = 0;
      //FIXME: Don't move particles over solid tiles
      particle->pos.x = new_x;
      particle->pos.y = new_y;
    }
  }
}

ObjectSettings
RainParticleSystem::get_settings() {
  ObjectSettings result(_("Rain particles"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &z_pos));

  return result;
}

/* EOF */
