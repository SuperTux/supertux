//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#include <config.h>

#include <iostream>
#include <cmath>

#include "particlesystem.h"
#include "app/globals.h"
#include "video/drawing_context.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"

ParticleSystem::ParticleSystem()
{
    virtual_width = screen->w;
    virtual_height = screen->h;
    layer = LAYER_BACKGROUND1;
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
  context.set_translation(Vector(0,0));
  
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        Particle* particle = *i;

        // remap x,y coordinates onto screencoordinates
        Vector pos;
        pos.x = fmodf(particle->pos.x - scrollx, virtual_width);
        if(pos.x < 0) pos.x += virtual_width;
        pos.y = fmodf(particle->pos.y - scrolly, virtual_height);
        if(pos.y < 0) pos.y += virtual_height;

        if(pos.x > screen->w) pos.x -= virtual_width;
        if(pos.y > screen->h) pos.y -= virtual_height;
        context.draw_surface(particle->texture, pos, layer);
    }

    context.pop_transform();
}

SnowParticleSystem::SnowParticleSystem()
{
    snowimages[0] = new Surface(datadir+"/images/shared/snow0.png", true);
    snowimages[1] = new Surface(datadir+"/images/shared/snow1.png", true);
    snowimages[2] = new Surface(datadir+"/images/shared/snow2.png", true);

    virtual_width = screen->w * 2;

    // create some random snowflakes
    size_t snowflakecount = size_t(virtual_width/10.0);
    for(size_t i=0; i<snowflakecount; ++i) {
        SnowParticle* particle = new SnowParticle;
        particle->pos.x = rand() % int(virtual_width);
        particle->pos.y = rand() % screen->h;
        int snowsize = rand() % 3;
        particle->texture = snowimages[snowsize];
        do {
            particle->speed = snowsize*.2 + (float(rand()%10)*.4);
        } while(particle->speed < 1);
        particle->speed *= 10; // gravity

        particles.push_back(particle);
    }
}

void
SnowParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("layer", layer);
}

void
SnowParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-snow");
  writer.write_int("layer", layer);
  writer.end_list("particles-snow");
}

SnowParticleSystem::~SnowParticleSystem()
{
  for(int i=0;i<3;++i)
    delete snowimages[i];
}

void SnowParticleSystem::action(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        SnowParticle* particle = (SnowParticle*) *i;
        particle->pos.y += particle->speed * elapsed_time;
        if(particle->pos.y > screen->h) {
            particle->pos.y = fmodf(particle->pos.y , virtual_height);
            particle->pos.x = rand() % int(virtual_width);
        }
    }
}

CloudParticleSystem::CloudParticleSystem()
{
    cloudimage = new Surface(datadir + "/images/shared/cloud.png", true);

    virtual_width = 2000.0;

    // create some random clouds
    for(size_t i=0; i<15; ++i) {
        CloudParticle* particle = new CloudParticle;
        particle->pos.x = rand() % int(virtual_width);
        particle->pos.y = rand() % int(virtual_height);
        particle->texture = cloudimage;
        particle->speed = -float(25 + rand() % 30);

        particles.push_back(particle);
    }
}

void
CloudParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("layer", layer);
}

void
CloudParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-clouds");
  writer.write_int("layer", layer);
  writer.end_list("particles-clouds");
}

CloudParticleSystem::~CloudParticleSystem()
{
  delete cloudimage;
}

void CloudParticleSystem::action(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        CloudParticle* particle = (CloudParticle*) *i;
        particle->pos.x += particle->speed * elapsed_time;
    }
}
