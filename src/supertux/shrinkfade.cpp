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

#include "supertux/globals.hpp"
#include "supertux/shrinkfade.hpp"
#include "video/drawing_context.hpp"

ShrinkFade::ShrinkFade(const Vector& dest_, float fade_time_) :
  dest(dest_),
  fade_time(fade_time_),
  accum_time(0),
  initial_size(SCREEN_HEIGHT > SCREEN_WIDTH ? SCREEN_HEIGHT : SCREEN_WIDTH)
{
}

ShrinkFade::~ShrinkFade()
{
}

void
ShrinkFade::update(float elapsed_time)
{
  accum_time += elapsed_time;
  if(accum_time > fade_time)
    accum_time = fade_time;
}

void
ShrinkFade::draw(DrawingContext& context)
{
  float progress = accum_time / fade_time;
  float diameter = 2 * initial_size * (1.0f - progress);
  context.draw_inverse_ellipse(dest, Vector(1.1f * diameter, diameter),
                               Color(0, 0, 0), LAYER_GUI+1);
}

bool
ShrinkFade::done() const
{
  return accum_time >= fade_time;
}

/* EOF */
