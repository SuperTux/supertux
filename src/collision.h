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

#ifndef SUPERTUX_COLLISION_H
#define SUPERTUX_COLLISION_H

#include "type.h"

class Tile;
class World;

/* Collision objects */
enum
{
  CO_BULLET,
  CO_BADGUY,
  CO_PLAYER
};

enum CollisionType {
    COLLISION_NORMAL,
    COLLISION_BUMP,
    COLLISION_SQUISH
};

bool rectcollision(const base_type& one, const base_type& two);
bool rectcollision_offset(const base_type& one, const base_type& two, float off_x, float off_y);

void collision_swept_object_map(base_type* old, base_type* current);
bool collision_object_map(const base_type& object);

/** Return a pointer to the tile at the given x/y coordinates */
Tile* gettile(float x, float y);

// Some little helper function to check for tile properties
bool  issolid(float x, float y);
bool  isbrick(float x, float y);
bool  isice(float x, float y);
bool  isfullbox(float x, float y);

typedef void* (*tiletestfunction)(Tile* tile);
/** invokes the function for each tile the baserectangle collides with. The
 * function aborts and returns true as soon as the tiletestfunction returns
 * != 0 then this value is returned. returns 0 if all tests failed.
 */
void* collision_func(const base_type& base, tiletestfunction* function);

Tile* collision_goal(const base_type& base);

#endif /*SUPERTUX_COLLISION_H*/

