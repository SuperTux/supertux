//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include "shrinkfade.hpp"
#include "main.hpp"
#include "video/drawing_context.hpp"

ShrinkFade::ShrinkFade(const Vector& dest, float fade_time)
  : dest(dest), fade_time(fade_time), accum_time(0)
{
  speedleft = dest.x / fade_time;
  speedright = (SCREEN_WIDTH - dest.x) / fade_time;
  speedtop = dest.y / fade_time;
  speedbottom = (SCREEN_HEIGHT - dest.y) / fade_time;
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
  context.draw_inverse_ellipse(dest,
                               Vector(2*SCREEN_WIDTH  * (1.0f - progress), 
                                      2*SCREEN_HEIGHT * (1.0f - progress)),
                               Color(0, 0, 0), LAYER_GUI+1);
}

bool
ShrinkFade::done()
{
  return accum_time >= fade_time;
}
