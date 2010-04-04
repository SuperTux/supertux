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

#include "supertux/collision.hpp"

#include <algorithm>

#include "math/aatriangle.hpp"
#include "math/rectf.hpp"

namespace collision {

bool intersects(const Rectf& r1, const Rectf& r2)
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

}

bool rectangle_aatriangle(Constraints* constraints, const Rectf& rect,
                          const AATriangle& triangle, const Vector& addl_ground_movement)
{
  if(!intersects(rect, (const Rectf&) triangle))
    return false;

  Vector normal;
  float c;
  Vector p1;
  Rectf area;
  switch(triangle.dir & AATriangle::DEFORM_MASK) {
    case 0:
      area.p1 = triangle.bbox.p1;
      area.p2 = triangle.bbox.p2;
      break;
    case AATriangle::DEFORM_BOTTOM:
      area.p1 = Vector(triangle.bbox.p1.x, triangle.bbox.p1.y + triangle.bbox.get_height()/2);
      area.p2 = triangle.bbox.p2;
      break;
    case AATriangle::DEFORM_TOP:
      area.p1 = triangle.bbox.p1;
      area.p2 = Vector(triangle.bbox.p2.x, triangle.bbox.p1.y + triangle.bbox.get_height()/2);
      break;
    case AATriangle::DEFORM_LEFT:
      area.p1 = triangle.bbox.p1;
      area.p2 = Vector(triangle.bbox.p1.x + triangle.bbox.get_width()/2, triangle.bbox.p2.y);
      break;
    case AATriangle::DEFORM_RIGHT:
      area.p1 = Vector(triangle.bbox.p1.x + triangle.bbox.get_width()/2, triangle.bbox.p1.y);
      area.p2 = triangle.bbox.p2;
      break;
    default:
      assert(false);
  }

  switch(triangle.dir & AATriangle::DIRECTION_MASK) {
    case AATriangle::SOUTHWEST:
      p1 = Vector(rect.p1.x, rect.p2.y);
      makePlane(area.p1, area.p2, normal, c);
      break;
    case AATriangle::NORTHEAST:
      p1 = Vector(rect.p2.x, rect.p1.y);
      makePlane(area.p2, area.p1, normal, c);
      break;
    case AATriangle::SOUTHEAST:
      p1 = rect.p2;
      makePlane(Vector(area.p1.x, area.p2.y),
                Vector(area.p2.x, area.p1.y), normal, c);
      break;
    case AATriangle::NORTHWEST:
      p1 = rect.p1;
      makePlane(Vector(area.p2.x, area.p1.y),
                Vector(area.p1.x, area.p2.y), normal, c);
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

  Vector outvec = normal * (depth + 0.2f);

  const float RDELTA = 3;
  if(p1.x < area.p1.x - RDELTA || p1.x > area.p2.x + RDELTA
     || p1.y < area.p1.y - RDELTA || p1.y > area.p2.y + RDELTA) {
    set_rectangle_rectangle_constraints(constraints, rect, area);
  } else {
    if(outvec.x < 0) {
      constraints->constrain_right(rect.get_right() + outvec.x, addl_ground_movement.x);
      constraints->hit.right = true;
    } else {
      constraints->constrain_left(rect.get_left() + outvec.x, addl_ground_movement.x);
      constraints->hit.left = true;
    }

    if(outvec.y < 0) {
      constraints->constrain_bottom(rect.get_bottom() + outvec.y, addl_ground_movement.y);
      constraints->hit.bottom = true;
      constraints->ground_movement += addl_ground_movement;
    } else {
      constraints->constrain_top(rect.get_top() + outvec.y, addl_ground_movement.y);
      constraints->hit.top = true;
    }
    constraints->hit.slope_normal = normal;
  }

  return true;
}

void set_rectangle_rectangle_constraints(Constraints* constraints,
                                         const Rectf& r1, const Rectf& r2, const Vector& addl_ground_movement)
{
  float itop = r1.get_bottom() - r2.get_top();
  float ibottom = r2.get_bottom() - r1.get_top();
  float ileft = r1.get_right() - r2.get_left();
  float iright = r2.get_right() - r1.get_left();

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if(vert_penetration < horiz_penetration) {
    if(itop < ibottom) {
      constraints->constrain_bottom(r2.get_top(), addl_ground_movement.y);
      constraints->hit.bottom = true;
      constraints->ground_movement += addl_ground_movement;
    } else {
      constraints->constrain_top(r2.get_bottom(), addl_ground_movement.y);
      constraints->hit.top = true;
    }
  } else {
    if(ileft < iright) {
      constraints->constrain_right(r2.get_left(), addl_ground_movement.x);
      constraints->hit.right = true;
    } else {
      constraints->constrain_left(r2.get_right(), addl_ground_movement.x);
      constraints->hit.left = true;
    }
  }
}

}

/* EOF */
