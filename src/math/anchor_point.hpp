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

#ifndef HEADER_SUPERTUX_MATH_ANCHOR_POINT_HPP
#define HEADER_SUPERTUX_MATH_ANCHOR_POINT_HPP

#include <string>
#include <vector>

#include "math/vector.hpp"

class Rectf;

enum AnchorPoint {
  ANCHOR_TOP_LEFT,
  ANCHOR_TOP,
  ANCHOR_TOP_RIGHT,
  ANCHOR_LEFT,
  ANCHOR_MIDDLE,
  ANCHOR_RIGHT,
  ANCHOR_BOTTOM_LEFT,
  ANCHOR_BOTTOM,
  ANCHOR_BOTTOM_RIGHT,
  ANCHOR_LAST = ANCHOR_BOTTOM_RIGHT
};

const std::vector<std::string> g_anchor_keys = {
  "topleft", "top", "topright", "left", "middle", "right", "bottomleft",
  "bottom", "bottomright"
};

std::vector<std::string> get_anchor_names();
std::string anchor_point_to_string(AnchorPoint point);
AnchorPoint string_to_anchor_point(const std::string& str);
Vector get_anchor_pos(const Rectf& rect, AnchorPoint point);
Vector get_anchor_pos(const Rectf& destrect, float width, float height,
                      AnchorPoint point);
Vector get_anchor_center_pos(const Rectf& rect, AnchorPoint point);

#endif

/* EOF */
