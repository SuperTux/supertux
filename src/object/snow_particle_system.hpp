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

#ifndef HEADER_SUPERTUX_OBJECT_SNOW_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_SNOW_PARTICLE_SYSTEM_HPP

#include "object/particlesystem.hpp"

class SnowParticleSystem : public ParticleSystem
{
public:
  SnowParticleSystem();
  virtual ~SnowParticleSystem();

  void parse(const Reader& lisp);

  virtual void update(float elapsed_time);

  std::string type() const
  { return "SnowParticleSystem"; }

private:
  class SnowParticle : public Particle
  {
  public:
    float speed;
    float wobble;
    float anchorx;
    float drift_speed;

    SnowParticle() :
      speed(),
      wobble(),
      anchorx(),
      drift_speed()
    {}
  };

  SurfacePtr snowimages[3];

private:
  SnowParticleSystem(const SnowParticleSystem&);
  SnowParticleSystem& operator=(const SnowParticleSystem&);
};

#endif

/* EOF */
