#include <algorithm>

#include "constraints.hpp"
#include "../math/aatriangle.hpp"
#include "../math/rectf.hpp"

namespace collision {

Constraints::Constraints() :
hit(),
position_left(),
position_right(),
position_top(),
position_bottom()
{
    float infinity = (std::numeric_limits<float>::has_infinity ?
                      std::numeric_limits<float>::infinity() :
                      std::numeric_limits<float>::max());
    position_left = -infinity;
    position_right = infinity;
    position_top = -infinity;
    position_bottom = infinity;
}

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
}