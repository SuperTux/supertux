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

#ifndef SUPERTUX_PARTICLESYSTEM_INTERACTIVE_H
#define SUPERTUX_PARTICLESYSTEM_INTERACTIVE_H

#include <vector>

#include "video/surface.hpp"
#include "game_object.hpp"
#include "util/serializable.hpp"
#include "sector.hpp"
#include "math/vector.hpp"

namespace lisp {
class Lisp;
}

class DisplayManager;

/**
 * This is an alternative class for particle systems. It is responsible for storing a
 * set of particles with each having an x- and y-coordinate the number of the
 * layer where it should be drawn and a texture.
 * This version of the particle system class doesn't use virtual screen coordinates,
 * but Interactive ones. Particle systems which need Interactive levels coordinates, such
 * as rain, should be implemented here.
 * Classes that implement a particle system should subclass from this class,
 * initialize particles in the constructor and move them in the simulate
 * function.
 */
class ParticleSystem_Interactive : public GameObject
{
public:
    ParticleSystem_Interactive();
    virtual ~ParticleSystem_Interactive();

    virtual void draw(DrawingContext& context);

protected:
    int z_pos;

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
    int collision(Particle* particle, Vector movement);
};

class RainParticleSystem : public ParticleSystem_Interactive, public Serializable
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

    Surface* rainimages[2];
};

class CometParticleSystem : public ParticleSystem_Interactive, public Serializable
{
public:
    CometParticleSystem();
    virtual ~CometParticleSystem();

    void parse(const lisp::Lisp& lisp);
    void write(lisp::Writer& writer);

    virtual void update(float elapsed_time);

    std::string type() const
    { return "CometParticleSystem"; }

private:
    class CometParticle : public Particle
    {
    public:
        float speed;
    };

    Surface* cometimages[2];
};

#endif
