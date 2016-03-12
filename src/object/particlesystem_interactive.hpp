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

#ifndef HEADER_SUPERTUX_OBJECT_PARTICLESYSTEM_INTERACTIVE_HPP
#define HEADER_SUPERTUX_OBJECT_PARTICLESYSTEM_INTERACTIVE_HPP

#include "math/vector.hpp"
#include "supertux/game_object.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"
#include "video/surface_ptr.hpp"

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

  virtual void parse(const ReaderMapping& reader);
  virtual void draw(DrawingContext& context);

  int get_layer() {
    return z_pos;
  }

protected:
  class Particle
  {
  public:
    Particle() :
      pos(),
      texture()
    {}

    virtual ~Particle()
    {}

    Vector pos;
    SurfacePtr texture;

  private:
    Particle(const Particle&);
    Particle& operator=(const Particle&);
  };

  int collision(Particle* particle, Vector movement);

  int z_pos;
  std::vector<Particle*> particles;
  float virtual_width;
  float virtual_height;
};

#endif

/* EOF */
