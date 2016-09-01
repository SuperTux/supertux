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

#include "object/particlesystem.hpp"

#include <math.h>

#include "math/random_generator.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"

ParticleSystem::ParticleSystem(float max_particle_size_) :
  ExposedObject<ParticleSystem, scripting::ParticleSystem>(this),
  max_particle_size(max_particle_size_),
  z_pos(),
  particles(),
  virtual_width(),
  virtual_height(),
  enabled(true)
{
  virtual_width = SCREEN_WIDTH + max_particle_size * 2;
  virtual_height = SCREEN_HEIGHT + max_particle_size *2;
  z_pos = LAYER_BACKGROUND1;
}

ObjectSettings
ParticleSystem::get_settings() {
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &z_pos,
                                         "z-pos"));

  result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
  return result;
}

void ParticleSystem::parse(const ReaderMapping& reader)
{
  if (!reader.get("name", name))
    name = "";
  if(!reader.get("enabled", enabled))
    enabled = true;
  z_pos = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND1);
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::draw(DrawingContext& context)
{
  if(!enabled)
    return;

  float scrollx = context.get_translation().x;
  float scrolly = context.get_translation().y;

  context.push_transform();
  context.set_translation(Vector(max_particle_size,max_particle_size));

  for(const auto& particle : particles) {
    // remap x,y coordinates onto screencoordinates
    Vector pos;

    pos.x = fmodf(particle->pos.x - scrollx, virtual_width);
    if(pos.x < 0) pos.x += virtual_width;

    pos.y = fmodf(particle->pos.y - scrolly, virtual_height);
    if(pos.y < 0) pos.y += virtual_height;

    //if(pos.x > virtual_width) pos.x -= virtual_width;
    //if(pos.y > virtual_height) pos.y -= virtual_height;

    context.draw_surface(particle->texture, pos, particle->angle, Color(1.0f, 1.0f, 1.0f), Blend(), z_pos);
  }

  context.pop_transform();
}

void
ParticleSystem::set_enabled(bool enabled_)
{
  enabled = enabled_;
}

bool
ParticleSystem::get_enabled() const
{
  return enabled;
}

/* EOF */
