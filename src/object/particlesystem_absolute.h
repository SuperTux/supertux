//  $Id: ParticleSystem_Absolute.h 2462 2005-05-10 15:38:16Z wansti $
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
#ifndef SUPERTUX_PARTICLESYSTEM_ABSOLUTE_H
#define SUPERTUX_PARTICLESYSTEM_ABSOLUTE_H

#include <vector>

#include "video/surface.h"
#include "game_object.h"
#include "serializable.h"
#include "sector.h"
#include "math/vector.h"

namespace lisp {
class Lisp;
}

class DisplayManager;

/**
 * This is the base class for particle systems. It is responsible for storing a
 * set of particles with each having an x- and y-coordinate the number of the
 * layer where it should be drawn and a texture.
 * The coordinate system used here is a virtual one. It would be a bad idea to
 * populate whole levels with particles. So we're using a virtual rectangle
 * here that is tiled onto the level when drawing. This rect.has the size
 * (virtual_width, virtual_height). We're using modulo on the particle
 * coordinates, so when a particle leaves left, it'll reenter at the right
 * side.
 *
 * Classes that implement a particle system should subclass from this class,
 * initialize particles in the constructor and move them in the simulate
 * function.
 */
class ParticleSystem_Absolute : public GameObject
{
public:
    ParticleSystem_Absolute();
    virtual ~ParticleSystem_Absolute();
    
    virtual void draw(DrawingContext& context);

protected:
    int layer;

    class Particle
    {
    public:
        virtual ~Particle()
        { }

        Vector pos;
        Surface* texture;
    };
    
    std::vector<Particle*> particles;
    float virtual_width, virtual_height;
};

class RainParticleSystem : public ParticleSystem_Absolute, public Serializable
{
public:
    RainParticleSystem();
    virtual ~RainParticleSystem();

    void parse(const lisp::Lisp& lisp);
    void write(lisp::Writer& writer);

    virtual void update(float elapsed_time);

    std::string type() const
    { return "RainParticleSystem"; }
    
private:
    class RainParticle : public Particle
    {
    public:
        float speed;
    };
    
    bool collision(RainParticle* particle, Vector movement);
    Surface* rainimages[2];
};

#endif

