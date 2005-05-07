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
#include "video/drawing_context.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "resources.h"
#include "main.h"

ParticleSystem::ParticleSystem()
{
    virtual_width = SCREEN_WIDTH;
    virtual_height = SCREEN_HEIGHT;
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

        if(pos.x > SCREEN_WIDTH) pos.x -= virtual_width;
        if(pos.y > SCREEN_HEIGHT) pos.y -= virtual_height;
        context.draw_surface(particle->texture, pos, layer);
    }

    context.pop_transform();
}

SnowParticleSystem::SnowParticleSystem()
{
    snowimages[0] = new Surface(datadir+"/images/objects/particles/snow0.png", true);
    snowimages[1] = new Surface(datadir+"/images/objects/particles/snow1.png", true);
    snowimages[2] = new Surface(datadir+"/images/objects/particles/snow2.png", true);

    virtual_width = SCREEN_WIDTH * 2;

    // create some random snowflakes
    size_t snowflakecount = size_t(virtual_width/10.0);
    for(size_t i=0; i<snowflakecount; ++i) {
        SnowParticle* particle = new SnowParticle;
        particle->pos.x = rand() % int(virtual_width);
        particle->pos.y = rand() % SCREEN_HEIGHT;
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
        if(particle->pos.y > SCREEN_HEIGHT) {
            particle->pos.y = fmodf(particle->pos.y , virtual_height);
            particle->pos.x = rand() % int(virtual_width);
        }
    }
}

RainParticleSystem::RainParticleSystem()
{
    rainimages[0] = new Surface(datadir+"/images/objects/particles/rain0.png", true);
    rainimages[1] = new Surface(datadir+"/images/objects/particles/rain1.png", true);

    virtual_width = SCREEN_WIDTH * 2;

    // create some random raindrops
    size_t raindropcount = size_t(virtual_width/8.0);
    for(size_t i=0; i<raindropcount; ++i) {
        RainParticle* particle = new RainParticle;
        particle->pos.x = rand() % int(virtual_width);
        particle->pos.y = rand() % SCREEN_HEIGHT;
        int rainsize = rand() % 2;
        particle->texture = rainimages[rainsize];
        do {
            particle->speed = (rainsize+1)*45 + (float(rand()%10)*.4);
        } while(particle->speed < 1);
        particle->speed *= 10; // gravity

        particles.push_back(particle);
    }
}

void
RainParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("layer", layer);
}

void
RainParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-rain");
  writer.write_int("layer", layer);
  writer.end_list("particles-rain");
}

RainParticleSystem::~RainParticleSystem()
{
  for(int i=0;i<2;++i)
    delete rainimages[i];
}

void RainParticleSystem::action(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        RainParticle* particle = (RainParticle*) *i;
        particle->pos.y += particle->speed * elapsed_time;
        particle->pos.x -= particle->speed * elapsed_time;
        if(particle->pos.y > SCREEN_HEIGHT) {
            particle->pos.y = fmodf(particle->pos.y , virtual_height);
            particle->pos.x = rand() % int(virtual_width);
        }
    }
}

CloudParticleSystem::CloudParticleSystem()
{
    cloudimage = new Surface(datadir + "/images/objects/particles/cloud.png", true);

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
