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

#ifndef SUPERTUX_PARTICLES_HPP
#define SUPERTUX_PARTICLES_HPP

#include "math/vector.hpp"
#include "game_object.hpp"
#include "timer.hpp"
#include "video/color.hpp"

class Particles : public GameObject
{
public:
  Particles(const Vector& epicenter, int min_angle, int max_angle,
            const Vector& initial_velocity, const Vector& acceleration,
            int number, Color color, int size, float life_time,
            int drawing_layer);
  ~Particles();

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector accel;
  Timer timer;
  bool live_forever;

  Color color;
  float size;
  int drawing_layer;

  struct Particle {
    Vector pos, vel;
//     float angle;
    };
  std::vector <Particle*> particles;
};

#endif
