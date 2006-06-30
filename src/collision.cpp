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
#include <config.h>

#include "collision.hpp"

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "math/vector.hpp"
#include "math/aatriangle.hpp"
#include "math/rect.hpp"
#include "collision_hit.hpp"
#include "log.hpp"

bool
Collision::intersects(const Rect& r1, const Rect& r2)
{
  if(r1.p2.x < r2.p1.x || r1.p1.x > r2.p2.x)
    return false;
  if(r1.p2.y < r2.p1.y || r1.p1.y > r2.p2.y)
    return false;

  return true;
}

//---------------------------------------------------------------------------

namespace {
  inline void makePlane(const Vector& p1, const Vector& p2, Vector& n, float& c)
  {
    n = Vector(p2.y-p1.y, p1.x-p2.x);
    c = -(p2 * n);
    float nval = n.norm();             
    n /= nval;
    c /= nval;
  }

  static const float DELTA = .0001;
}

bool
Collision::rectangle_aatriangle(Constraints* constraints, const Rect& rect,
    const AATriangle& triangle)
{
  if(!intersects(rect, (const Rect&) triangle))
    return false;

  Vector normal;
  float c;
  Vector p1;
  Vector tp1, tp2;
  switch(triangle.dir & AATriangle::DEFORM_MASK) {
    case 0:
      tp1 = triangle.p1;
      tp2 = triangle.p2;
      break;
    case AATriangle::DEFORM1:
      tp1 = Vector(triangle.p1.x, triangle.p1.y + triangle.get_height()/2);
      tp2 = triangle.p2;
      break;
    case AATriangle::DEFORM2:
      tp1 = triangle.p1;
      tp2 = Vector(triangle.p2.x, triangle.p1.y + triangle.get_height()/2);
      break;
    case AATriangle::DEFORM3:
      tp1 = triangle.p1;
      tp2 = Vector(triangle.p1.x + triangle.get_width()/2, triangle.p2.y);
      break;
    case AATriangle::DEFORM4:
      tp1 = Vector(triangle.p1.x + triangle.get_width()/2, triangle.p1.y);
      tp2 = triangle.p2;
      break;
    default:
      assert(false);
  } 
  
  switch(triangle.dir & AATriangle::DIRECTION_MASK) {
    case AATriangle::SOUTHWEST:
      if(rect.get_left() < triangle.get_left())
        return false;
      p1 = Vector(rect.p1.x, rect.p2.y);
      makePlane(tp1, tp2, normal, c);
      break;
    case AATriangle::NORTHEAST:
      if(rect.get_right() > triangle.get_right())
        return false;
      p1 = Vector(rect.p2.x, rect.p1.y);
      makePlane(tp2, tp1, normal, c);
      break;
    case AATriangle::SOUTHEAST:
      if(rect.get_right() > triangle.get_right())
        return false;
      p1 = rect.p2;
      makePlane(Vector(tp1.x, tp2.y),
          Vector(tp2.x, tp1.y), normal, c);
      break;
    case AATriangle::NORTHWEST:
      if(rect.get_left() < triangle.get_left())
        return false;
      p1 = rect.p1;
      makePlane(Vector(tp2.x, tp1.y),
          Vector(tp1.x, tp2.y), normal, c);
      break;
    default:
      assert(false);
  }

  float n_p1 = -(normal * p1);
  float depth = n_p1 - c;
  if(depth < 0)
    return false;

#if 0
  std::cout << "R: " << rect << " Tri: " << triangle << "\n";
  std::cout << "Norm: " << normal << " Depth: " << depth << "\n";
#endif

  Vector outvec = normal * depth;
  if(outvec.x < 0) {
    constraints->right = rect.get_right() + outvec.x;
  } else {
    constraints->left = rect.get_left() + outvec.x;
  }
  if(outvec.y < 0) {
    constraints->bottom = rect.get_bottom() + outvec.y;
    constraints->hit.bottom = true;
  } else {
    constraints->top = rect.get_top() + outvec.y;
    constraints->hit.top = true;
  }

  return true;
}

