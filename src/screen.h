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
#include "texture.h"

#define NO_UPDATE false
#define UPDATE true
#define USE_ALPHA 0
#define IGNORE_ALPHA 1

struct Color
{
  Color() 
    : red(0), green(0), blue(0)
  {}
  
  Color(int red_, int green_, int blue_)
    : red(red_), green(green_), blue(blue_)
  {}

  int red, green, blue;
};

void drawline(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
void clearscreen(int r, int g, int b);
void drawgradient(Color top_clr, Color bot_clr);
void fillrect(float x, float y, float w, float h, int r, int g, int b, int a);
//void black_fade(Surface* surface, int seconds, bool fade_out);
void fade(const std::string& surface, int seconds, bool fade_out);
void updatescreen(void);
void flipscreen(void);
void update_rect(SDL_Surface *scr, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
void fadeout();

#endif /*SUPERTUX_SCREEN_H*/
