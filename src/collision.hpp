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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __COLLISION_H__
#define __COLLISION_H__

class Vector;
class Rect;
class AATriangle;
class CollisionHit;
class Constraints;

class Collision
{
public:
  /** checks if 2 rectangle intersect each other */
  static bool intersects(const Rect& r1, const Rect& r2);
  
  /** does collision detection between a rectangle and an axis aligned triangle
   * Returns true in case of a collision and fills in the hit structure then.
   */                                                                         
  static bool rectangle_aatriangle(Constraints* constraints, const Rect& rect,
                                   const AATriangle& triangle);
};

#endif

