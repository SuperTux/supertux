#include <config.h>

#include "collision.h"

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "math/vector.h"
#include "math/aatriangle.h"
#include "math/rectangle.h"
#include "collision_hit.h"

namespace SuperTux
{

bool
Collision::rectangle_rectangle(CollisionHit& hit, const Rectangle& r1,
    const Rectangle& r2)
{
  if(r1.p2.x < r2.p1.x || r1.p1.x > r2.p2.x)
    return false;
  if(r1.p2.y < r2.p1.y || r1.p1.y > r2.p2.y)
    return false;
  
  hit.depth = r1.p2.x - r2.p1.x;
  hit.normal.x = -1;
  hit.normal.y = 0;

  float px2 = r2.p2.x - r1.p1.x;
  if(px2 < hit.depth) {
    hit.depth = px2;
    hit.normal.x = 1;
    hit.normal.y = 0;
  }

  float py1 = r1.p2.y - r2.p1.y;
  if(py1 < hit.depth) {
    hit.depth = py1;
    hit.normal.x = 0;
    hit.normal.y = -1;
  }

  float py2 = r2.p2.y - r1.p1.y;
  if(py2 < hit.depth) {
    hit.depth = py2;
    hit.normal.x = 0;
    hit.normal.y = 1;
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
Collision::rectangle_aatriangle(CollisionHit& hit, const Rectangle& rect,
    const AATriangle& triangle)
{
  if(!rectangle_rectangle(hit, rect, (const Rectangle&) triangle))
    return false;
  
  Vector normal;
  float c;
  Vector p1;
  switch(triangle.dir) {
    case AATriangle::SOUTHWEST:
      p1 = Vector(rect.p1.x, rect.p2.y);
      makePlane(triangle.p1, triangle.p2, normal, c);
      break;
    case AATriangle::NORTHEAST:
      p1 = Vector(rect.p2.x, rect.p1.y);
      makePlane(triangle.p2, triangle.p1, normal, c);
      break;
    case AATriangle::SOUTHEAST:
      p1 = rect.p2;
      makePlane(Vector(triangle.p1.x, triangle.p2.y),
          Vector(triangle.p2.x, triangle.p1.y), normal, c);
      break;
    case AATriangle::NORTHWEST:
      p1 = rect.p1;
      makePlane(Vector(triangle.p2.x, triangle.p1.y),
          Vector(triangle.p1.x, triangle.p2.y), normal, c);
      break;
  }

  float depth = -(normal * p1) - c;
  if(depth < 0)
    return false;
  if(depth < hit.depth) {
    hit.depth = depth;
    hit.normal = normal;
  }

  return true;
}

}
