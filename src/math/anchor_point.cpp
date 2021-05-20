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

#include "math/anchor_point.hpp"

#include <config.h>

#include <stdexcept>
#include <sstream>

#include "math/rectf.hpp"
#include "util/log.hpp"

std::string anchor_point_to_string(AnchorPoint point)
{
  switch (point) {
    case ANCHOR_TOP_LEFT:
      return "topleft";
    case ANCHOR_TOP:
      return "top";
    case ANCHOR_TOP_RIGHT:
      return "topright";
    case ANCHOR_LEFT:
      return "left";
    case ANCHOR_MIDDLE:
      return "middle";
    case ANCHOR_RIGHT:
      return "right";
    case ANCHOR_BOTTOM_LEFT:
      return "bottomleft";
    case ANCHOR_BOTTOM:
      return "bottom";
    case ANCHOR_BOTTOM_RIGHT:
      return "bottomright";
    default:
      throw std::runtime_error("Invalid anchor point");
  }
}

AnchorPoint string_to_anchor_point(const std::string& str)
{
  if (str == "topleft")
    return ANCHOR_TOP_LEFT;
  else if (str == "top")
    return ANCHOR_TOP;
  else if (str == "topright")
    return ANCHOR_TOP_RIGHT;
  else if (str == "left")
    return ANCHOR_LEFT;
  else if (str == "middle")
    return ANCHOR_MIDDLE;
  else if (str == "right")
    return ANCHOR_RIGHT;
  else if (str == "bottomleft")
    return ANCHOR_BOTTOM_LEFT;
  else if (str == "bottom")
    return ANCHOR_BOTTOM;
  else if (str == "bottomright")
    return ANCHOR_BOTTOM_RIGHT;

  std::ostringstream msg;
  msg << "Unknown anchor '" << str << "'";
  throw std::runtime_error(msg.str());
}

Vector get_anchor_pos(const Rectf& rect, AnchorPoint point)
{
  Vector result(0.0f, 0.0f);

  switch (point & ANCHOR_V_MASK) {
    case ANCHOR_LEFT:
      result.x = rect.get_left();
      break;
    case ANCHOR_MIDDLE:
      result.x = rect.get_left() + (rect.get_right() - rect.get_left()) / 2.0f;
      break;
    case ANCHOR_RIGHT:
      result.x = rect.get_right();
      break;
    default:
      log_warning << "Invalid anchor point found" << std::endl;
      result.x = rect.get_left();
      break;
  }

  switch (point & ANCHOR_H_MASK) {
    case ANCHOR_TOP:
      result.y = rect.get_top();
      break;
    case ANCHOR_MIDDLE:
      result.y = rect.get_top() + (rect.get_bottom() - rect.get_top()) / 2.0f;
      break;
    case ANCHOR_BOTTOM:
      result.y = rect.get_bottom();
      break;
    default:
      log_warning << "Invalid anchor point found" << std::endl;
      result.y = rect.get_top();
      break;
  }

  return result;
}

Vector get_anchor_pos(const Rectf& destrect, float width, float height,
                      AnchorPoint point)
{
  Vector result(0.0f, 0.0f);

  switch (point & ANCHOR_V_MASK) {
    case ANCHOR_LEFT:
      result.x = destrect.get_left();
      break;
    case ANCHOR_MIDDLE:
      result.x = destrect.get_middle().x - width / 2.0f;
      break;
    case ANCHOR_RIGHT:
      result.x = destrect.get_right() - width;
      break;
    default:
      log_warning << "Invalid anchor point found" << std::endl;
      result.x = destrect.get_left();
      break;
  }

  switch (point & ANCHOR_H_MASK) {
    case ANCHOR_TOP:
      result.y = destrect.get_top();
      break;
    case ANCHOR_MIDDLE:
      result.y = destrect.get_middle().y - height / 2.0f;
      break;
    case ANCHOR_BOTTOM:
      result.y = destrect.get_bottom() - height;
      break;
    default:
      log_warning << "Invalid anchor point found" << std::endl;
      result.y = destrect.get_top();
      break;
  }

  return result;
}

/* EOF */
