#ifndef __COLLISION_H__
#define __COLLISION_H__

namespace SuperTux
{

class Rectangle;
class CollisionHit;
class AATriangle;

class Collision
{
public:
  /** does collision detection between 2 rectangles. Returns true in case of
   * collision and fills in the hit structure then.
   */
  static bool rectangle_rectangle(CollisionHit& hit, const Rectangle& r1,
      const Rectangle& r2);

  /** does collision detection between a rectangle and an axis aligned triangle
   * Returns true in case of a collision and fills in the hit structure then.
   */                                                                         
  static bool rectangle_aatriangle(CollisionHit& hit, const Rectangle& rect,
      const AATriangle& triangle);                                            
};

}

#endif

