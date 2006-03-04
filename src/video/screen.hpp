//  $Id: screen.h 1850 2004-08-27 20:34:56Z rmcruz $
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
#include <iostream>

#include <vector>
#include "math/vector.hpp"

void fillrect(float x, float y, float w, float h, int r, int g, int b, int a = 255);

void fadeout(int fade_time);
void shrink_fade(const Vector& point, int fade_time);

#endif
