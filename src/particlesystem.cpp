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

#include "particlesystem.h"

#include <iostream>
#include <math.h>
#include "globals.h"
#include "world.h"
#include "level.h"
#include "scene.h"

ParticleSystem::ParticleSystem()
{
#ifndef RES320X240
    virtual_width = screen->w;
    virtual_height = screen->h;
#else
    virtual_width = 640;
    virtual_height = 480;
#endif
}

ParticleSystem::~ParticleSystem()
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        delete *i;
    }
}

void ParticleSystem::draw(float scrollx, float scrolly, int layer)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        Particle* particle = *i;
        if(particle->layer != layer)
            continue;
        
        // remap x,y coordinates onto screencoordinates
        float x = fmodf(particle->x - scrollx, virtual_width);
        if(x < 0) x += virtual_width;
        float y = fmodf(particle->y - scrolly, virtual_height);
        if(y < 0) y += virtual_height;
        float xmax = fmodf(x + particle->texture->w, virtual_width);
        float ymax = fmodf(y + particle->texture->h, virtual_height);

        // particle on screen
#ifndef RES320X240
        if(x >= screen->w && xmax >= screen->w)
            continue;
        if(y >= screen->h && ymax >= screen->h)
            continue;
        
        if(x > screen->w) x -= virtual_width;
        if(y > screen->h) y -= virtual_height;
#else
        if(x >= 640 && xmax >= 640)
            continue;
        if(y >= 480 && ymax >= 480)
            continue;

        if(x > 640) x -= virtual_width;
        if(y > 480) y -= virtual_height;
#endif
        
        particle->texture->draw(x, y);
    }
}

SnowParticleSystem::SnowParticleSystem()
{
    snowimages[0] = new Surface(datadir+"/images/shared/snow0.png", USE_ALPHA);
    snowimages[1] = new Surface(datadir+"/images/shared/snow1.png", USE_ALPHA);
    snowimages[2] = new Surface(datadir+"/images/shared/snow2.png", USE_ALPHA);

#ifndef RES320X240
    virtual_width = screen->w * 2;

#else
    virtual_width = 640 * 2;
#endif
    // create some random snowflakes
    size_t snowflakecount = size_t(virtual_width/10.0);
    for(size_t i=0; i<snowflakecount; ++i) {
        SnowParticle* particle = new SnowParticle;
        particle->x = rand() % int(virtual_width);
#ifndef RES320X240
        particle->y = rand() % screen->h;
#else
        particle->y = rand() % 480;
#endif
        particle->layer = i % 2;
        int snowsize = rand() % 3;
        particle->texture = snowimages[snowsize];
        do {
            particle->speed = snowsize/60.0 + (float(rand()%10)/300.0);
        } while(particle->speed < 0.01);
        particle->speed *= World::current()->get_level()->gravity;

        particles.push_back(particle);
    }
}

SnowParticleSystem::~SnowParticleSystem()
{
  for(int i=0;i<3;++i)
    delete snowimages[i];
}

void SnowParticleSystem::simulate(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        SnowParticle* particle = (SnowParticle*) *i;
        particle->y += particle->speed * elapsed_time;
#ifndef RES320X240
        if(particle->y > screen->h) {
#else
        if(particle->y > 480) {
#endif
            particle->y = fmodf(particle->y , virtual_height);
            particle->x = rand() % int(virtual_width);
        }
    }
}

CloudParticleSystem::CloudParticleSystem()
{
    cloudimage = new Surface(datadir + "/images/shared/cloud.png", USE_ALPHA);

    virtual_width = 2000.0;

    // create some random clouds
    for(size_t i=0; i<15; ++i) {
        CloudParticle* particle = new CloudParticle;
        particle->x = rand() % int(virtual_width);
        particle->y = rand() % int(virtual_height);
        particle->layer = 0;
        particle->texture = cloudimage;
        particle->speed = -float(250 + rand() % 200) / 1000.0;

        particles.push_back(particle);
    }
}

CloudParticleSystem::~CloudParticleSystem()
{
  delete cloudimage;
}

void CloudParticleSystem::simulate(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        CloudParticle* particle = (CloudParticle*) *i;
        particle->x += particle->speed * elapsed_time;
    }
}
