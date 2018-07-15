//  SuperTux - Pulsing Light
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "object/pulsing_light.hpp"

#include <assert.h>
#include <math.h>

#include "math/random_generator.hpp"

PulsingLight::PulsingLight(const Vector& center, float cycle_len_, float min_alpha_, float max_alpha_, const Color& color_) :
  Light(center, color_),
  min_alpha(min_alpha_),
  max_alpha(max_alpha_),
  cycle_len(cycle_len_),
  t(0)
{
  assert(cycle_len > 0);

  // start with random phase offset
  t = gameRandom.randf(0.0, cycle_len);
}

PulsingLight::~PulsingLight()
{
}

void
PulsingLight::update(float elapsed_time)
{
  Light::update(elapsed_time);

  t += elapsed_time;
  if (t > cycle_len) t -= cycle_len;
}

void
PulsingLight::draw(DrawingContext& context)
{
  Color old_color = color;

  color.alpha *= min_alpha + ((max_alpha - min_alpha) * cos(2 * M_PI * t / cycle_len));
  Light::draw(context);

  color = old_color;
}

/* EOF */
