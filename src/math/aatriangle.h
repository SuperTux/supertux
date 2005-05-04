#ifndef __AATRIANGLE_H__
#define __AATRIANGLE_H__

#include "rectangle.h"

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
   *
   * Deform flags: (see docs/aatriangletypes.png for details)
   */
  enum Direction {
    SOUTHWEST = 0,
    NORTHEAST,
    SOUTHEAST,
    NORTHWEST,
    DIRECTION_MASK = 0x0003,
    DEFORM1 = 0x0010,
    DEFORM2 = 0x0020,
    DEFORM3 = 0x0030,
    DEFORM4 = 0x0040,
    DEFORM_MASK = 0x0070
  };

  AATriangle()
    : dir(SOUTHWEST)
  {
  }
  AATriangle(const Vector& v1, const Vector& v2, int newdir)
    : Rectangle(v1, v2), dir(newdir)
  {
  }

  int dir;
};

#endif

