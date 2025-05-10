#ifndef HEADER_SUPERTUX_COLLISION_CONSTRAINTS_HPP
#define HEADER_SUPERTUX_COLLISION_CONSTRAINTS_HPP

#include <limits>
#include <algorithm>

#include "collision_hit.hpp"
#include "../math/fwd.hpp"

class Rectf;
class AATriangle;

namespace collision {

class Constraints final
{
public:
  Constraints();

  bool has_constraints() const
  {
    float infinity = (std::numeric_limits<float>::has_infinity ?
                      std::numeric_limits<float>::infinity() :
                      std::numeric_limits<float>::max());
    return
      position_left   > -infinity ||
      position_right  <  infinity ||
      position_top    > -infinity ||
      position_bottom <  infinity;
  }

public:

  void constrain_left (float position)
  {
    position_left = std::max (position_left, position);
  }

  void constrain_right (float position)
  {
    position_right = std::min (position_right, position);
  }

  void constrain_top (float position)
  {
    position_top = std::max (position_top, position);
  }

  void constrain_bottom (float position)
  {
    position_bottom = std::min (position_bottom, position);
  }

  void merge_constraints (const Constraints& other);

  float get_position_left   () const { return position_left;   }
  float get_position_right  () const { return position_right;  }
  float get_position_top    () const { return position_top;    }
  inline float get_position_bottom () const { return position_bottom; }

  inline float get_height () const { return (position_bottom - position_top); }
  inline float get_width  () const { return (position_right - position_left); }

  inline float get_x_midpoint () const { return (.5f * (position_left + position_right)); }

  CollisionHit hit;

private:
  float position_left;
  float position_right;
  float position_top;
  float position_bottom;
};

}

#endif