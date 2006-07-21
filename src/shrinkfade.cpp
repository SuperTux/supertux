//  $Id$
//
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
  Color black(0, 0, 0);
  float left = speedleft * accum_time;
  float top = speedtop * accum_time;
  float right = SCREEN_WIDTH - speedright * accum_time;
  float bottom = SCREEN_HEIGHT - speedbottom * accum_time;

  context.draw_filled_rect(Vector(0, 0),
                           Vector(left, SCREEN_HEIGHT),
                           black, LAYER_GUI+1);
  context.draw_filled_rect(Vector(0, 0),
                           Vector(SCREEN_WIDTH, top),
                           black, LAYER_GUI+1);
  context.draw_filled_rect(Vector(right, 0),
                           Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                           black, LAYER_GUI+1);
  context.draw_filled_rect(Vector(0, bottom),
                           Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                           black, LAYER_GUI+1);
}

bool
ShrinkFade::done()
{
  return accum_time >= fade_time;
}
