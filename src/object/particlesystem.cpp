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

#include <config.h>

#include <iostream>
#include <cmath>

#include "particlesystem.hpp"
#include "video/drawing_context.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "supertux/resources.hpp"
#include "supertux/main.hpp"
#include "object/camera.hpp"
#include "math/random_generator.hpp"

ParticleSystem::ParticleSystem(float max_particle_size)
        : max_particle_size(max_particle_size)
{
  virtual_width = SCREEN_WIDTH + max_particle_size * 2;
  virtual_height = SCREEN_HEIGHT + max_particle_size *2;
  z_pos = LAYER_BACKGROUND1;
}

ParticleSystem::~ParticleSystem()
{
  std::vector<Particle*>::iterator i;
  for(i = particles.begin(); i != particles.end(); ++i) {
    delete *i;
  }
}

void ParticleSystem::draw(DrawingContext& context)
{
  float scrollx = context.get_translation().x;
  float scrolly = context.get_translation().y;

  context.push_transform();
  context.set_translation(Vector(max_particle_size,max_particle_size));

  std::vector<Particle*>::iterator i;
  for(i = particles.begin(); i != particles.end(); ++i) {
    Particle* particle = *i;

    // remap x,y coordinates onto screencoordinates
    Vector pos;

    pos.x = fmodf(particle->pos.x - scrollx, virtual_width);
    if(pos.x < 0) pos.x += virtual_width;

    pos.y = fmodf(particle->pos.y - scrolly, virtual_height);
    if(pos.y < 0) pos.y += virtual_height;


    //if(pos.x > virtual_width) pos.x -= virtual_width;
    //if(pos.y > virtual_height) pos.y -= virtual_height;

    context.draw_surface(particle->texture, pos, z_pos);
  }

  context.pop_transform();
}

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
SnowParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("z-pos", z_pos);
}

void
SnowParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-snow");
  writer.write("z-pos", z_pos);
  writer.end_list("particles-snow");
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
GhostParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("z-pos", z_pos);
}

void
GhostParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-ghosts");
  writer.write("z-pos", z_pos);
  writer.end_list("particles-ghosts");
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

CloudParticleSystem::CloudParticleSystem()
        : ParticleSystem(128)
{
  cloudimage = new Surface("images/objects/particles/cloud.png");

  virtual_width = 2000.0;

  // create some random clouds
  for(size_t i=0; i<15; ++i) {
    CloudParticle* particle = new CloudParticle;
    particle->pos.x = systemRandom.rand(static_cast<int>(virtual_width));
    particle->pos.y = systemRandom.rand(static_cast<int>(virtual_height));
    particle->texture = cloudimage;
    particle->speed = -systemRandom.randf(25.0, 54.0);

    particles.push_back(particle);
  }
}

void
CloudParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("z-pos", z_pos);
}

void
CloudParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-clouds");
  writer.write("z-pos", z_pos);
  writer.end_list("particles-clouds");
}

CloudParticleSystem::~CloudParticleSystem()
{
  delete cloudimage;
}

void CloudParticleSystem::update(float elapsed_time)
{
  std::vector<Particle*>::iterator i;
  for(i = particles.begin(); i != particles.end(); ++i) {
    CloudParticle* particle = (CloudParticle*) *i;
    particle->pos.x += particle->speed * elapsed_time;
  }
}
