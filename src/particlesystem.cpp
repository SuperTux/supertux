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
#include "scene.h"

ParticleSystem::ParticleSystem()
{
    virtual_width = screen->w;
    virtual_height = screen->h;
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
        if(x >= screen->w && xmax >= screen->w)
            continue;
        if(y >= screen->h && ymax >= screen->h)
            continue;
        
        if(x > screen->w) x -= virtual_width;
        if(y > screen->h) y -= virtual_height;
        texture_draw(particle->texture, x, y);
    }
}

SnowParticleSystem::SnowParticleSystem()
{
    texture_load(&snowimages[0], datadir+"/images/shared/snow0.png", USE_ALPHA);
    texture_load(&snowimages[1], datadir+"/images/shared/snow1.png", USE_ALPHA);
    texture_load(&snowimages[2], datadir+"/images/shared/snow2.png", USE_ALPHA);

    virtual_width = screen->w * 2;

    // create some random snowflakes
    size_t snowflakecount = size_t(virtual_width/10.0);
    for(size_t i=0; i<snowflakecount; ++i) {
        SnowParticle* particle = new SnowParticle;
        particle->x = rand() % int(virtual_width);
        particle->y = rand() % screen->h;
        particle->layer = i % 2;
        int snowsize = rand() % 3;
        particle->texture = &snowimages[snowsize];
        particle->speed = 0.01 + snowsize/50.0 + (rand()%(int)gravity/15.0);

        particles.push_back(particle);
    }
}

SnowParticleSystem::~SnowParticleSystem()
{
    for(int i=0;i<3;++i)
        texture_free(&snowimages[i]);
}

void SnowParticleSystem::simulate(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        SnowParticle* particle = (SnowParticle*) *i;
        particle->y += particle->speed * elapsed_time;
        if(particle->y > screen->h) {
            particle->y = 0;
            particle->x = rand() % int(virtual_width);
        }
    }
}

CloudParticleSystem::CloudParticleSystem()
{
    texture_load(&cloudimage, datadir + "/images/shared/cloud.png", USE_ALPHA);

    virtual_width = 5000.0;

    // create some random clouds
    for(size_t i=0; i<15; ++i) {
        CloudParticle* particle = new CloudParticle;
        particle->x = rand() % int(virtual_width);
        particle->y = rand() % int((float) screen->h * 0.3333);
        particle->layer = 0;
        particle->texture = &cloudimage;
        particle->speed = -float(250 + rand() % 200) / 1000.0;

        particles.push_back(particle);
    }
}

CloudParticleSystem::~CloudParticleSystem()
{
    texture_free(&cloudimage);
}

void CloudParticleSystem::simulate(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        CloudParticle* particle = (CloudParticle*) *i;
        particle->x += particle->speed * elapsed_time;
    }
}
