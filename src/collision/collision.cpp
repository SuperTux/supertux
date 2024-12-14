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

#include "collision/collision.hpp"

#include <algorithm>
#include <iostream>

#include "math/aatriangle.hpp"
#include "math/rectf.hpp"

namespace collision {

void Constraints::merge_constraints(const Constraints& other)
{
  constrain_left(other.position_left);
  constrain_right(other.position_right);
  constrain_top(other.position_top);
  constrain_bottom(other.position_bottom);

  hit.left |= other.hit.left;
  hit.right |= other.hit.right;
  hit.top |= other.hit.top;
  hit.bottom |= other.hit.bottom;
  hit.crush |= other.hit.crush;
}

//---------------------------------------------------------------------------

namespace {
inline float dot(const Vector& p1, const Vector& p2)
{
  return p1.x*p2.y+p1.y*p2.y;
}
inline void makePlane(const Vector& p1, const Vector& p2, Vector& n, float& c)
{
  n = Vector(p2.y - p1.y, p1.x - p2.x);
  c = -(p2 * n);
  float nval = n.length();
  n /= nval;
  c /= nval;
}

/*
bool
rectangle_aatriangle_deform_rect(Constraints* constraints, const Rectf& rect,
                                 const AATriangle& triangle, const Rectf& trirect)
{
  // This is the half of the triangl
  Rectf halftri;
  halftri.set_size(trirect.get_width(), trirect.get_width());
  halftri.set_pos({ triangle.bbox.get_left(), triangle.bbox.get_top() });

  // Check for collision on the other part of the deformed triangle
  if (triangle.is_east() &&
      triangle.get_deform() == AATriangle::DEFORM_RIGHT)
  {
    halftri.set_left(halftri.get_left() + trirect.get_width());
  }
  else if (!triangle.is_east() &&
           triangle.get_deform() == AATriangle::DEFORM_LEFT)
  {
    halftri.set_right(halftri.get_right() - trirect.get_width());
  }
  else if (triangle.is_north() &&
      triangle.get_deform() == AATriangle::DEFORM_BOTTOM)
  {
    halftri.set_bottom(halftri.get_bottom() - trirect.get_height());
  }
  else if (!triangle.is_north() &&
           triangle.get_deform() == AATriangle::DEFORM_TOP)
  {
    halftri.set_top(halftri.get_top() + trirect.get_height());
  }
  else
  {
    return false;
  }

  set_rectangle_rectangle_constraints(constraints, rect, halftri);

  if (constraints->has_constraints())
    return true;

  /*
  if (triangle.is_east())
  {
    if (!math::in_bounds(rect.p1().y, trip1.y - rect.get_height(), trip2.y) ||
        rect.p1().x < trip2.x)
      goto continue_collision;

    constraints->hit.left = true;
    constraints->constrain_left(trip2.x);
  }
    if (!math::in_bounds(rect.p1().y, trip1.y - rect.get_height(), trip2.y) ||
        rect.p1().x < trip2.x)
      goto continue_collision;

    constraints->hit.left = true;
    constraints->constrain_left(trip2.x);
  }
  else
  {
    if (!math::in_bounds(rect.p1().y, trip1.y - rect.get_height(), trip2.y) ||
        rect.p1().x >= trip1.x)
      goto continue_collision;

    constraints->hit.right = true;
    constraints->constrain_right(trip1.x);
  }

  if (triangle.is_north())
  {
    if (!math::in_bounds(rect.p1().x, trip1.x - rect.get_width(), trip2.x) ||
        rect.p1().y >= trip1.y)
      goto continue_collision;

    constraints->hit.bottom = true;
    hits_rectangle_bottom = true;
    constraints->constrain_bottom(trip2.y);
  }
  else
  {
    if (!math::in_bounds(rect.p1().x, trip1.x - rect.get_width(), trip2.x) ||
        rect.p1().y < trip2.y)
      goto continue_collision;

    constraints->hit.top = true;
    constraints->constrain_top(trip1.y);
  }

  return false;
}
*/

}

bool
rectangle_aatriangle(Constraints* constraints, const Rectf& rect,
                     const AATriangle& triangle)
{
  bool dummy;
  return rectangle_aatriangle(constraints, rect, triangle, dummy);
}

bool rectangle_aatriangle(Constraints* constraints, const Rectf& rect,
                          const AATriangle& triangle,
                          bool& hits_rectangle_bottom)
{
  // Welcome to the slope collision algorithm. Enjoy your stay!

#if 1
  // For more information on the two following variables, read
  // https://github.com/SuperTux/supertux/wiki/Tileset#slope-types

  // The deform represents the shape of the slope ("normal", "steep", "gentle").
  int deform = triangle.get_deform();

  // The direction represents the orientation of the slope.
  int dir = triangle.get_dir();

  // These are both the angles in the triangle.
  // angx is the angle on the left or right, aligned with the 90º angle.
  // angy is the angle at the top or bottom, aligned with the 90º angle.
  float angx, angy;

  // This is the bounding box of the triangle, which can be different depending on the
  // deform value.
  Rectf trirect;

  switch (deform)
  {
    case 0:
      trirect = triangle.bbox;
      angx = 45.f;
      angy = 45.f;
      break;

    case AATriangle::DEFORM_BOTTOM:
      trirect.set_p1(Vector(triangle.bbox.get_left(), triangle.bbox.get_top() + triangle.bbox.get_height()/2));
      trirect.set_p2(triangle.bbox.p2());

      angx = triangle.is_north() ? 26.56f : 63.43f;
      angy = triangle.is_north() ? 63.43f : 26.56f;

      break;

    case AATriangle::DEFORM_TOP:
      trirect.set_p1(triangle.bbox.p1());
      trirect.set_p2(Vector(triangle.bbox.get_right(), triangle.bbox.get_top() + triangle.bbox.get_height()/2));

      angx = triangle.is_north() ? 26.56f : 63.43f;
      angy = triangle.is_north() ? 63.43f : 26.56f;

      break;

    case AATriangle::DEFORM_LEFT:
      trirect.set_p1(triangle.bbox.p1());
      trirect.set_p2(Vector(triangle.bbox.get_left() + triangle.bbox.get_width()/2, triangle.bbox.get_bottom()));

      angx = triangle.is_north() ? 63.43f : 26.56f;
      angy = triangle.is_north() ? 26.56f : 63.43f;

      break;

    case AATriangle::DEFORM_RIGHT:
      trirect.set_p1(Vector(triangle.bbox.get_left() + triangle.bbox.get_width()/2, triangle.bbox.get_top()));
      trirect.set_p2(triangle.bbox.p2());

      angx = triangle.is_north() ? 63.43f : 26.56f;
      angy = triangle.is_north() ? 26.56f : 63.43f;

      break;

    default:
      assert(false);
  }

  if (!trirect.overlaps(rect))
    return false;

  Vector trip1 = trirect.p1();
  Vector trip2 = trirect.p2();
  Sizef trisz = trirect.get_size();

  // ar represents the vertice of the triangle that contains the 90º angle
  // sp represents the same thing but relative to rect instead of trirect
  Vector ar, sp, diff;

  switch (dir)
  {
    case AATriangle::SOUTHWEST:
      ar = { trip1.x, trip2.y };
      sp = { rect.p1().x, rect.p2().y };
      diff = { sp.x - ar.x, ar.y - sp.y };
      break;
    case AATriangle::NORTHEAST:
      ar = { trip2.x, trip1.y };
      sp = { rect.p2().x, rect.p1().y };
      diff = { ar.x - sp.x, sp.y - ar.y };
      break;
    case AATriangle::SOUTHEAST:
      ar = trip2;
      sp = rect.p2();
      diff = ar - sp;
      break;
    case AATriangle::NORTHWEST:
      ar = trip1;
      sp = rect.p1();
      diff = sp - ar;
      break;
    default:
      assert(false);
  }

  /*
  if (triangle.get_deform() != 0 &&
      rectangle_aatriangle_deform_rect(constraints, rect, triangle, trirect))
  {
    return true;
  }
  */



  float ratio = 1.f;
  if (trisz.height > 0.f)
    ratio = std::abs(trisz.width / trisz.height);

  float sum = diff.x + (diff.y * ratio);

  // If colliding with the vertice that contains the x angle,
  // check if ar.y is above sp.y in a south triangle or below sp.y in a north triangle.
  // If it isn't, ignore. This prevents "teleporting" to the triangle when touching its ledge.
  if (diff.y * ratio <= 0.f)
  {
    if (triangle.is_east())
      constraints->hit.right = true;
    else
      constraints->hit.left = true;

    return true;
  }

  // Do something similar for the x axis
  if (diff.x <= 0.f)
  {
    if (triangle.is_north())
    {
      constraints->hit.top = true;
      constraints->constrain_top(trirect.get_bottom());
    }
    else
    {
      hits_rectangle_bottom = true;
      constraints->hit.bottom = true;
      constraints->constrain_bottom(trirect.get_top());
    }

    return true;
  }

  float trisum = (trisz.width + (trisz.height * ratio)) / 2;
  if (sum <= trisum)
  {
    //dx+((dy+dc)*ratio)=area
    float dc = (trisum - (diff.y * ratio) - diff.x) / ratio;
    if (dc + diff.y > trisz.height)
    {
      dc = trisz.height - diff.y;
    }

    /*
    if (triangle.is_north())
    {
      //dc = -dc;
    }
    //dc -= 1.f;

    //std::cout << "dc: " << dc << " sp: " << sp << std::endl;
    */

    if (triangle.is_east())
    {
      constraints->hit.right = true;
      constraints->hit.slope_normal.x = -1;
      constraints->constrain_right(sp.x - dc);
    }
    else
    {
      constraints->hit.left = true;
      constraints->hit.slope_normal.x = 1;
      constraints->constrain_left(sp.x + dc);
    }

    if (triangle.is_north())
    {
      constraints->hit.top = true;
      constraints->hit.slope_normal.y = 1;
      constraints->constrain_top(sp.y + dc);
    }
    else
    {
      constraints->hit.bottom = true;
      hits_rectangle_bottom = true;
      constraints->hit.slope_normal.y = -1;
      constraints->constrain_bottom(sp.y - dc);
    }

    constraints->hit.slope_normal.x *= std::cos(angx);
    constraints->hit.slope_normal.y *= std::sin(angy);

    return true;
  }

  return false;
#else
  if (!rect.overlaps(triangle.bbox))
      return false;

    Vector normal(0.0f, 0.0f);
    float c = 0.0;
    Vector p1(0.0f, 0.0f);
    Rectf area;
    switch (triangle.dir & AATriangle::DEFORM_MASK) {
      case 0:
        area.set_p1(triangle.bbox.p1());
        area.set_p2(triangle.bbox.p2());
        break;
      case AATriangle::DEFORM_BOTTOM:
        area.set_p1(Vector(triangle.bbox.get_left(), triangle.bbox.get_top() + triangle.bbox.get_height()/2));
        area.set_p2(triangle.bbox.p2());
        break;
      case AATriangle::DEFORM_TOP:
        area.set_p1(triangle.bbox.p1());
        area.set_p2(Vector(triangle.bbox.get_right(), triangle.bbox.get_top() + triangle.bbox.get_height()/2));
        break;
      case AATriangle::DEFORM_LEFT:
        area.set_p1(triangle.bbox.p1());
        area.set_p2(Vector(triangle.bbox.get_left() + triangle.bbox.get_width()/2, triangle.bbox.get_bottom()));
        break;
      case AATriangle::DEFORM_RIGHT:
        area.set_p1(Vector(triangle.bbox.get_left() + triangle.bbox.get_width()/2, triangle.bbox.get_top()));
        area.set_p2(triangle.bbox.p2());
        break;
      default:
        assert(false);
    }

    switch (triangle.dir & AATriangle::DIRECTION_MASK) {
      case AATriangle::SOUTHWEST:
        p1 = Vector(rect.get_left(), rect.get_bottom());
        makePlane(area.p1(), area.p2(), normal, c);
        break;
      case AATriangle::NORTHEAST:
        p1 = Vector(rect.get_right(), rect.get_top());
        makePlane(area.p2(), area.p1(), normal, c);
        break;
      case AATriangle::SOUTHEAST:
        p1 = rect.p2();
        makePlane(Vector(area.get_left(), area.get_bottom()),
                  Vector(area.get_right(), area.get_top()), normal, c);
        break;
      case AATriangle::NORTHWEST:
        p1 = rect.p1();
        makePlane(Vector(area.get_right(), area.get_top()),
                  Vector(area.get_left(), area.get_bottom()), normal, c);
        break;
      default:
        assert(false);
    }

    float n_p1 = -(normal * p1);
    float depth = n_p1 - c;
    if (depth < 0)
      return false;

  #if 1
    std::cout << "R: " << rect << " Tri: " << triangle.get_dir() + triangle.get_deform() << "\n";
    std::cout << "Norm: " << normal << " Depth: " << depth << std::endl;
  #endif

    Vector outvec = normal * (depth + 0.3f);

    const float RDELTA = 3;
    if (p1.x < area.get_left() - RDELTA || p1.x > area.get_right() + RDELTA
       || p1.y < area.get_top() - RDELTA || p1.y > area.get_bottom() + RDELTA) {
      set_rectangle_rectangle_constraints(constraints, rect, area);
    } else {
      if (outvec.x < 0) {
        constraints->constrain_right(rect.get_right() + outvec.x);
        constraints->hit.right = true;
      } else {
        constraints->constrain_left(rect.get_left() + outvec.x);
        constraints->hit.left = true;
      }

      if (outvec.y < 0) {
        constraints->constrain_bottom(rect.get_bottom() + outvec.y);
        constraints->hit.bottom = true;
        hits_rectangle_bottom = true;
      } else {
        constraints->constrain_top(rect.get_top() + outvec.y);
        constraints->hit.top = true;
      }
      constraints->hit.slope_normal = normal;
    }

    return true;
#endif
}

void set_rectangle_rectangle_constraints(Constraints* constraints, const Rectf& r1, const Rectf& r2)
{
  float itop = r1.get_bottom() - r2.get_top();
  float ibottom = r2.get_bottom() - r1.get_top();
  float ileft = r1.get_right() - r2.get_left();
  float iright = r2.get_right() - r1.get_left();

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if (vert_penetration < horiz_penetration) {
    if (itop < ibottom) {
      constraints->constrain_bottom(r2.get_top());
      constraints->hit.bottom = true;
    } else {
      constraints->constrain_top(r2.get_bottom());
      constraints->hit.top = true;
    }
  } else {
    if (ileft < iright) {
      constraints->constrain_right(r2.get_left());
      constraints->hit.right = true;
    } else {
      constraints->constrain_left(r2.get_right());
      constraints->hit.left = true;
    }
  }
}

bool line_intersects_line(const Vector& line1_start, const Vector& line1_end, const Vector& line2_start, const Vector& line2_end)
{
  // Adapted from Striker, (C) 1999 Joris van der Hoeven, GPL

  float a1 = line1_start.x, b1 = line1_start.y, a2 = line1_end.x, b2 = line1_end.y;
  float c1 = line2_start.x, d1 = line2_start.y, c2 = line2_end.x, d2 = line2_end.y;

  float num = (b2-b1)*(c2-c1) - (a2-a1)*(d2-d1);
  float den1 = (d2-b2)*(c1-c2) + (a2-c2)*(d1-d2);
  float den2 = (d2-b2)*(a1-a2) + (a2-c2)*(b1-b2);

  // Normalize to positive numerator.
  if (num < 0) {
    num = -num;
    den1 = -den1;
    den2 = -den2;
  }

  // Numerator is zero -> Check for parallel or coinciding lines.
  if (num == 0) {
    if ((b1-b2)*(c1-a2) != (a1-a2)*(d1-b2)) return false;
    if (a1 == a2) {
      std::swap(a1, b1);
      std::swap(a2, b2);
      std::swap(c1, d1);
      std::swap(c2, d2);
    }
    if (a1 > a2) std::swap(a1, a2);
    if (c1 > c2) std::swap(c1, c2);
    return ((a1 <= c2) && (a2 >= c1));
  }

  // Standard check.
  return (den1>=0) && (den1<=num) && (den2>=0) && (den2<=num);

}

bool intersects_line(const Rectf& r, const Vector& line_start, const Vector& line_end)
{
  Vector p1 = r.p1();
  Vector p2 = Vector(r.get_right(), r.get_top());
  Vector p3 = r.p2();
  Vector p4 = Vector(r.get_left(), r.get_bottom());
  if (line_intersects_line(p1, p2, line_start, line_end)) return true;
  if (line_intersects_line(p2, p3, line_start, line_end)) return true;
  if (line_intersects_line(p3, p4, line_start, line_end)) return true;
  if (line_intersects_line(p4, p1, line_start, line_end)) return true;
  return false;
}

}

/* EOF */
