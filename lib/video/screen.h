//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
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

#ifndef SUPERTUX_SCREEN_H
#define SUPERTUX_SCREEN_H

#include <SDL.h>
#ifndef NOOPENGL
#include <SDL_opengl.h>
#endif
#include <iostream>
class Color
{
public:
  Color() 
    : red(0), green(0), blue(0), alpha(255)
  {}
  
  Color(Uint8 red_, Uint8 green_, Uint8 blue_, Uint8 alpha_ = 255)
    : red(red_), green(green_), blue(blue_), alpha(alpha_)
  {}

  Color(const Color& o)
    : red(o.red), green(o.green), blue(o.blue), alpha(o.alpha)
  { }

  bool operator==(const Color& o)
    {  if(red == o.red && green == o.green &&
          blue == o.blue && alpha == o.alpha)
         return true;
       return false;  }

  Uint8 red, green, blue, alpha;
};

#include "video/surface.h"

class Vector;

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void drawpixel(int x, int y, Uint32 pixel);
void fillrect(float x, float y, float w, float h, int r, int g, int b, int a = 255);
void draw_line(float x1, float y1, float x2, int r, int g, int b, int a = 255);

void fadeout(int fade_time);
void shrink_fade(const Vector& point, int fade_time);

#endif /*SUPERTUX_SCREEN_H*/
