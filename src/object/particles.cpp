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

#include "object/particles.hpp"

#include <math.h>

#include "math/random_generator.hpp"
#include "object/camera.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"

//TODO: remove this function in favor of the one below
Particles::Particles(const Vector& epicenter, int min_angle, int max_angle,
                     const Vector& initial_velocity, const Vector& acceleration, int number,
                     Color color_, int size_, float life_time, int drawing_layer_) :
  accel(acceleration),
  timer(),
  live_forever(),
  color(color_),
  size(size_),
  drawing_layer(drawing_layer_),
  particles()
{
  if(life_time == 0) {
    live_forever = true;
  } else {
    live_forever = false;
    timer.start(life_time);
  }

  // create particles
  for(int p = 0; p < number; p++)
  {
    auto particle = std::unique_ptr<Particle>(new Particle);
    particle->pos = epicenter;

    float angle = graphicsRandom.rand(min_angle, max_angle)
      * (M_PI / 180);  // convert to radius (radians?)
    particle->vel.x = /*fabs*/(sin(angle)) * initial_velocity.x;
    //    if(angle >= M_PI && angle < M_PI*2)
    //      particle->vel.x *= -1;  // work around to fix signal
    particle->vel.y = /*fabs*/(cos(angle)) * initial_velocity.y;
    //    if(angle >= M_PI_2 && angle < 3*M_PI_2)
    //      particle->vel.y *= -1;

    particles.push_back(std::move(particle));
  }
}

Particles::Particles(const Vector& epicenter, int min_angle, int max_angle,
                     const float min_initial_velocity, const float max_initial_velocity,
                     const Vector& acceleration, int number, Color color_,
                     int size_, float life_time, int drawing_layer_) :

  accel(acceleration),
  timer(),
  live_forever(),
  color(color_),
  size(size_),
  drawing_layer(drawing_layer_),
  particles()
{
  if(life_time == 0) {
    live_forever = true;
  } else {
    live_forever = false;
    timer.start(life_time);
  }

  // create particles
  for(int p = 0; p < number; p++)
  {
    auto particle = std::unique_ptr<Particle>(new Particle);
    particle->pos = epicenter;

    float velocity = (min_initial_velocity == max_initial_velocity) ? min_initial_velocity :
                     graphicsRandom.rand(min_initial_velocity, max_initial_velocity);
    float angle = (min_angle == max_angle) ? min_angle * (M_PI / 180) :
                     graphicsRandom.rand(min_angle, max_angle) * (M_PI / 180);  // convert to radians
    // Note that angle defined as clockwise from vertical (up is zero degrees, right is 90 degrees)
    particle->vel.x = (sin(angle)) * velocity;
    particle->vel.y = (-cos(angle)) * velocity;

    particles.push_back(std::move(particle));
  }
}

Particles::~Particles()
{
}

void
Particles::update(float elapsed_time)
{
  Vector camera = Sector::current()->camera->get_translation();

  // update particles
  for(auto i = particles.begin(); i != particles.end(); ) {
    (*i)->pos.x += (*i)->vel.x * elapsed_time;
    (*i)->pos.y += (*i)->vel.y * elapsed_time;

    (*i)->vel.x += accel.x * elapsed_time;
    (*i)->vel.y += accel.y * elapsed_time;

    if((*i)->pos.x < camera.x || (*i)->pos.x > SCREEN_WIDTH + camera.x ||
       (*i)->pos.y < camera.y || (*i)->pos.y > SCREEN_HEIGHT + camera.y) {
      i = particles.erase(i);
    } else {
      ++i;
    }
  }

  if((timer.check() && !live_forever) || particles.size() == 0)
    remove_me();
}

void
Particles::draw(DrawingContext& context)
{
  // draw particles
  for(auto i = particles.begin(); i != particles.end(); ++i) {
    context.draw_filled_rect((*i)->pos, Vector(size,size), color,drawing_layer);
  }
}

/* EOF */
