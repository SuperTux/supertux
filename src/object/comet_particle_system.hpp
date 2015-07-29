//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_COMET_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_COMET_PARTICLE_SYSTEM_HPP

#include <memory>

#include "object/particlesystem_interactive.hpp"
#include "video/surface_ptr.hpp"
#include "video/drawing_request.hpp" /* LAYER_BACKGROUND1 */

class Writer;

class CometParticleSystem : public ParticleSystem_Interactive
{
public:
  CometParticleSystem();
  virtual ~CometParticleSystem();

  void write(Writer& writer);

  virtual void update(float elapsed_time);

  std::string type() const
  { return "CometParticleSystem"; }
  virtual std::string get_class() {
    return "particles-comets";
  }
private:
  class CometParticle : public Particle
  {
  public:
    float speed;

    CometParticle() :
      speed()
    {}
  };

  SurfacePtr cometimages[2];

private:
  CometParticleSystem(const CometParticleSystem&);
  CometParticleSystem& operator=(const CometParticleSystem&);
};

#endif

/* EOF */
