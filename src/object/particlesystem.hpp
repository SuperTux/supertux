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

#ifndef HEADER_SUPERTUX_OBJECT_PARTICLESYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_PARTICLESYSTEM_HPP

#include <vector>

#include "math/vector.hpp"
#include "squirrel/exposed_object.hpp"
#include "scripting/particlesystem.hpp"
#include "supertux/game_object.hpp"
#include "video/surface_ptr.hpp"

class ReaderMapping;

/**
  This is the base class for particle systems. It is responsible for
  storing a set of particles with each having an x- and y-coordinate
  the number of the layer where it should be drawn and a texture.

  The coordinate system used here is a virtual one. It would be a bad
  idea to populate whole levels with particles. So we're using a
  virtual rectangle here that is tiled onto the level when drawing.
  This rect.has the size (virtual_width, virtual_height). We're using
  modulo on the particle coordinates, so when a particle leaves left,
  it'll reenter at the right side.

  Classes that implement a particle system should subclass from this
  class, initialize particles in the constructor and move them in the
  simulate function.
 */
class ParticleSystem : public GameObject,
                       public ExposedObject<ParticleSystem, scripting::ParticleSystem>
{
public:
  ParticleSystem(const ReaderMapping& reader, float max_particle_size = 60);
  ParticleSystem(float max_particle_size = 60);
  ~ParticleSystem() override;

  virtual void draw(DrawingContext& context) override;

  virtual std::string get_class() const override { return "particle-system"; }
  virtual std::string get_display_name() const override { return _("Particle system"); }
  virtual ObjectSettings get_settings() override;

  void set_enabled(bool enabled_);
  bool get_enabled() const;

  int get_layer() const { return z_pos; }

protected:
  class Particle
  {
  public:
    Particle() :
      pos(0.0f, 0.0f),
      angle(),
      texture(),
      alpha(),
      scale(1.f) // This currently only works in the custom particle system
    {}

    virtual ~Particle()
    {}

    Vector pos;
    // angle at which to draw particle
    float angle;
    SurfacePtr texture;
    float alpha;
    float scale; // see initializer

  private:
    Particle(const Particle&) = delete;
    Particle& operator=(const Particle&) = delete;
  };

protected:
  float max_particle_size;
  int z_pos;
  std::vector<std::unique_ptr<Particle> > particles;
  float virtual_width;
  float virtual_height;
  bool enabled;

private:
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;
};

#endif

/* EOF */
