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

#ifndef SUPERTUX_PARTICLESYSTEM_H
#define SUPERTUX_PARTICLESYSTEM_H

#include <vector>
#include "texture.h"

/**
 * This is the base class for particle systems. It is responsible for storing a
 * set of particles with each having an x- and y-coordinate the number of the
 * layer where it should be drawn and a texture.
 * The coordinate system used here is a virtual one. It would be a bad idea to
 * populate whole levels with particles. So we're using a virtual rectangle
 * here that is tiled onto the level when drawing. This rectangle has the size
 * (virtual_width, virtual_height). We're using modulo on the particle
 * coordinates, so when a particle leaves left, it'll reenter at the right
 * side.
 *
 * Classes that implement a particle system should subclass from this class,
 * initialize particles in the constructor and move them in the simulate
 * function.
 */
class ParticleSystem
{
public:
    ParticleSystem();
    virtual ~ParticleSystem();
    
    void draw(float scrollx, float scrolly, int layer);

    virtual void simulate(float elapsed_time) = 0;

protected:
    class Particle
    {
    public:
        virtual ~Particle()
        { }

        float x, y;
        int layer;
        Surface* texture;
    };
    
    std::vector<Particle*> particles;
    float virtual_width, virtual_height;
};

class SnowParticleSystem : public ParticleSystem
{
public:
    SnowParticleSystem();
    virtual ~SnowParticleSystem();

    virtual void simulate(float elapsed_time);
    
private:
    class SnowParticle : public Particle
    {
    public:
        float speed;
    };
    
    Surface* snowimages[3];
};

class CloudParticleSystem : public ParticleSystem
{
public:
    CloudParticleSystem();
    virtual ~CloudParticleSystem();

    virtual void simulate(float elapsed_time);
    
private:
    class CloudParticle : public Particle
    {
    public:
        float speed;
    };
    
    Surface* cloudimage;
};

#endif

