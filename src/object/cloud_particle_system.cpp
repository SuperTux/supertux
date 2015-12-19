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
#include "video/drawing_context.hpp"
#include "util/gettext.hpp"

CloudParticleSystem::CloudParticleSystem() :
  ParticleSystem(128),
  cloudimage(Surface::create("images/objects/particles/cloud.png"))
{
  virtual_width = 2000.0;

  // create some random clouds
  for(size_t i=0; i<15; ++i) {
    CloudParticle* particle = new CloudParticle;
    particle->pos.x = graphicsRandom.rand(static_cast<int>(virtual_width));
    particle->pos.y = graphicsRandom.rand(static_cast<int>(virtual_height));
    particle->texture = cloudimage;
    particle->speed = -graphicsRandom.randf(25.0, 54.0);

    particles.push_back(particle);
  }
}

void
CloudParticleSystem::parse(const Reader& reader)
{
  z_pos = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND1);
}

CloudParticleSystem::~CloudParticleSystem()
{
}

void CloudParticleSystem::update(float elapsed_time)
{
  std::vector<Particle*>::iterator i;
  for(i = particles.begin(); i != particles.end(); ++i) {
    CloudParticle* particle = (CloudParticle*) *i;
    particle->pos.x += particle->speed * elapsed_time;
  }
}

ObjectSettings
CloudParticleSystem::get_settings() {
  ObjectSettings result(_("Cloud particles"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &z_pos));

  return result;
}

/* EOF */
