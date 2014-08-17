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

#include "supertux/fadeout.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"

FadeOut::FadeOut(float fade_time_, Color color_)
  : color(color_), fade_time(fade_time_), accum_time(0)
{
}

FadeOut::~FadeOut()
{
}

void
FadeOut::update(float elapsed_time)
{
  accum_time += elapsed_time;
  if(accum_time > fade_time)
    accum_time = fade_time;
}

void
FadeOut::draw(DrawingContext& context)
{
  Color col = color;
  col.alpha = accum_time / fade_time;
  context.draw_filled_rect(Vector(0, 0),
                           Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                           col, LAYER_GUI+1);
}

bool
FadeOut::done()
{
  return accum_time >= fade_time;
}

/* EOF */
