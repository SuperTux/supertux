//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef SUPERTUX_TYPE_H
#define SUPERTUX_TYPE_H

#include <string>

#include "SDL.h"

/// 'Base' type for game objects.
/** Mainly for layered use in game objects.
    Containts upper left X and Y coordinates of an
    object along with its width and height. */
struct base_type
{
  float x;
  float y;
  float width;
  float height;
};


#endif /*SUPERTUX_TYPE_H*/

