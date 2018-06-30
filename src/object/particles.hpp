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

#ifndef HEADER_SUPERTUX_OBJECT_PARTICLES_HPP
#define HEADER_SUPERTUX_OBJECT_PARTICLES_HPP

#include <memory>

#include "math/vector.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"
#include "video/color.hpp"

class Particles : public GameObject
{
public:
  Particles(const Vector& epicenter, int min_angle, int max_angle,
            const Vector& initial_velocity, const Vector& acceleration,
            int number, Color color, int size, float life_time,
            int drawing_layer);
  Particles(const Vector& epicenter, int min_angle, int max_angle,
            const float min_initial_velocity, const float max_initial_velocity,
            const Vector& acceleration, int number, Color color,
            int size, float life_time, int drawing_layer);
  virtual bool is_saveable() const {
    return false;
  }

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  struct Particle {
    Vector pos, vel;

    Particle() :
      pos(),
      vel()
    {}
    //     float angle;
  };

private:
  Vector accel;
  Timer timer;
  bool live_forever;

  Color color;
  float size;
  int drawing_layer;

  std::vector<std::unique_ptr<Particle> > particles;
};

#endif

/* EOF */
