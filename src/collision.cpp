//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include "collision.h"

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "math/vector.h"
#include "math/aatriangle.h"
#include "math/rect.h"
#include "collision_hit.h"

static const float DELTA = .0001;

bool
Collision::rectangle_rectangle(CollisionHit& hit, const Rect& r1,
    const Vector& movement, const Rect& r2)
{
  if(r1.p2.x < r2.p1.x || r1.p1.x > r2.p2.x)
    return false;
  if(r1.p2.y < r2.p1.y || r1.p1.y > r2.p2.y)
    return false;

  if(movement.x > DELTA) {
    hit.depth = r1.p2.x - r2.p1.x;
    hit.time = hit.depth / movement.x;
    hit.normal.x = -1;
    hit.normal.y = 0;
  } else if(movement.x < -DELTA) {
    hit.depth = r2.p2.x - r1.p1.x;
    hit.time = hit.depth / -movement.x;
    hit.normal.x = 1;
    hit.normal.y = 0;
  } else {
    if(movement.y > -DELTA && movement.y < DELTA) {
      hit.time = 0;
      hit.depth = 0;
      hit.normal.x = 1;
      hit.normal.y = 0;
      return true;
    }
    hit.time = FLT_MAX;
  }

  if(movement.y > DELTA) {
    float ydepth = r1.p2.y - r2.p1.y;
    float yt = ydepth / movement.y;
    if(yt < hit.time) {
      hit.depth = ydepth;
      hit.time = yt;
      hit.normal.x = 0;
      hit.normal.y = -1;
    }
  } else if(movement.y < -DELTA) {
    float ydepth = r2.p2.y - r1.p1.y;
    float yt = ydepth / -movement.y;
    if(yt < hit.time) {
      hit.depth = ydepth;
      hit.time = yt;
      hit.normal.x = 0;
      hit.normal.y = 1;
    }
  }

  return true;
}

//---------------------------------------------------------------------------

static void makePlane(const Vector& p1, const Vector& p2, Vector& n, float& c)
{
  n = Vector(p2.y-p1.y, p1.x-p2.x);
  c = -(p2 * n);
  float nval = n.norm();             
  n /= nval;
  c /= nval;
}

bool
Collision::rectangle_aatriangle(CollisionHit& hit, const Rect& rect,
    const Vector& movement, const AATriangle& triangle)
{
  if(!rectangle_rectangle(hit, rect, movement, (const Rect&) triangle))
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
      p1 = Vector(rect.p1.x, rect.p2.y);
      makePlane(tp1, tp2, normal, c);
      break;
    case AATriangle::NORTHEAST:
      p1 = Vector(rect.p2.x, rect.p1.y);
      makePlane(tp2, tp1, normal, c);
      break;
    case AATriangle::SOUTHEAST:
      p1 = rect.p2;
      makePlane(Vector(tp1.x, tp2.y),
          Vector(tp2.x, tp1.y), normal, c);
      break;
    case AATriangle::NORTHWEST:
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
  float time = depth / -(normal * movement);
  if(time < hit.time) {
    hit.depth = depth;
    hit.time = time;
    hit.normal = normal;
  }

  return true;
}

