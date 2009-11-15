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
#ifndef HEADER_SUPERTUX_SUPERTUX_SHRINKFADE_HPP
#define HEADER_SUPERTUX_SUPERTUX_SHRINKFADE_HPP

#include "supertux/screen_fade.hpp"
#include "math/vector.hpp"

/**
 * Shrinks a rectangle screen towards a specific position
 */
class ShrinkFade : public ScreenFade
{
public:
  ShrinkFade(const Vector& point, float fade_time);
  virtual ~ShrinkFade();

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

  virtual bool done();

private:
  Vector dest;
  float fade_time;
  float accum_time;
  float speedleft, speedright, speedtop, speedbottom;
};

#endif
