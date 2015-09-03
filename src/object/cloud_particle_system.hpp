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

#ifndef HEADER_SUPERTUX_OBJECT_CLOUD_PARTICLE_SYTEM_HPP
#define HEADER_SUPERTUX_OBJECT_CLOUD_PARTICLE_SYTEM_HPP

#include <memory>

#include "object/particlesystem.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface_ptr.hpp"

class CloudParticleSystem : public ParticleSystem
{
public:
  CloudParticleSystem();
  virtual ~CloudParticleSystem();

  virtual void update(float elapsed_time);

  std::string type() const
  { return "CloudParticleSystem"; }
  virtual std::string get_class() const {
    return "particles-clouds";
  }

  virtual const std::string get_icon_path() const {
    return "images/engine/editor/clouds.png";
  }

private:
  class CloudParticle : public Particle
  {
  public:
    float speed;

    CloudParticle() :
      speed()
    {}
  };

  SurfacePtr cloudimage;

  virtual ObjectSettings get_settings();

private:
  CloudParticleSystem(const CloudParticleSystem&);
  CloudParticleSystem& operator=(const CloudParticleSystem&);
};

#endif

/* EOF */
