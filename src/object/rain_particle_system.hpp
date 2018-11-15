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

#ifndef HEADER_SUPERTUX_OBJECT_RAIN_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_RAIN_PARTICLE_SYSTEM_HPP

#include "object/particlesystem_interactive.hpp"
#include "video/surface_ptr.hpp"

class RainParticleSystem final : public ParticleSystem_Interactive
{
public:
  RainParticleSystem();
  RainParticleSystem(const ReaderMapping& reader);
  virtual ~RainParticleSystem();

  void init();
  virtual void update(float dt_sec) override;

  virtual std::string get_class() const override { return "particles-rain"; }
  virtual std::string get_display_name() const override { return _("Rain particles"); }

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/rain.png";
  }

private:
  class RainParticle : public Particle
  {
  public:
    float speed;

    RainParticle() :
      speed()
    {}
  };

  SurfacePtr rainimages[2];

private:
  RainParticleSystem(const RainParticleSystem&) = delete;
  RainParticleSystem& operator=(const RainParticleSystem&) = delete;
};

#endif

/* EOF */
