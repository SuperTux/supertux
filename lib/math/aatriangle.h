#ifndef __AATRIANGLE_H__
#define __AATRIANGLE_H__

#include "rectangle.h"

namespace SuperTux
{

/**
 * An axis aligned triangle (ie. a triangle where 2 sides are parallel to the x-
 * and y-axis.
 */
class AATriangle : public Rectangle
{
public:
  /** Directions:
   *
   *    SOUTHEWEST    NORTHEAST   SOUTHEAST    NORTHWEST
   *    *      or      *---*   or      *    or *---* 
   *    | \             \  |         / |       |  /
   *    |  \             \ |        /  |       | /
   *    *---*              *       *---*       *
   */
  enum Direction {
    SOUTHWEST, NORTHEAST, SOUTHEAST, NORTHWEST
  };

  AATriangle()
    : dir(SOUTHWEST)
  {
  }
  AATriangle(const Vector& v1, const Vector& v2, Direction newdir)
    : Rectangle(v1, v2), dir(newdir)
  {
  }

  Direction dir;
};

}

#endif

