#ifndef __ANCHOR_POINT_HPP__
#define __ANCHOR_POINT_HPP__

#include <string>
#include "math/vector.hpp"

class Rect;

enum AnchorPoint {
  ANCHOR_H_MASK = 0x00f0,
  ANCHOR_TOP    = 0x0010,
  ANCHOR_BOTTOM = 0x0020,
  ANCHOR_V_MASK = 0x000f,
  ANCHOR_LEFT   = 0x0001,
  ANCHOR_RIGHT  = 0x0002,
  ANCHOR_MIDDLE = 0x0000,
  
  ANCHOR_TOP_LEFT = ANCHOR_TOP | ANCHOR_LEFT,
  ANCHOR_TOP_RIGHT = ANCHOR_TOP | ANCHOR_RIGHT,
  ANCHOR_BOTTOM_LEFT = ANCHOR_BOTTOM | ANCHOR_LEFT,
  ANCHOR_BOTTOM_RIGHT = ANCHOR_BOTTOM | ANCHOR_RIGHT,
};

std::string anchor_point_to_string(AnchorPoint point);
AnchorPoint string_to_anchor_point(const std::string& str);
Vector get_anchor_pos(const Rect& rect, AnchorPoint point);
Vector get_anchor_pos(const Rect& destrect, float width, float height,
                      AnchorPoint point);

#endif

