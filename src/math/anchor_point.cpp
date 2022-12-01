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
#include "util/gettext.hpp"

std::vector<std::string> get_anchor_names()
{
  // Must be dynamic because language can change at runtime (+ tinygettext must
  // be init'ed)
  return {
    _("Top Left"),
    _("Top"),
    _("Top Right"),
    _("Left"),
    _("Middle"),
    _("Right"),
    _("Bottom Left"),
    _("Bottom"),
    _("Bottom Right"),
  };
}

std::string anchor_point_to_string(AnchorPoint point)
{
  try
  {
    return g_anchor_keys.at(point);
  }
  catch(const std::out_of_range&)
  {
    throw std::runtime_error("Invalid anchor point");
  }
}

AnchorPoint string_to_anchor_point(const std::string& str)
{
  for (size_t i = 0; i < g_anchor_keys.size(); i++)
    if (g_anchor_keys.at(i) == str)
      return static_cast<AnchorPoint>(i);

  std::ostringstream msg;
  msg << "Unknown anchor '" << str << "'";
  throw std::runtime_error(msg.str());
}

Vector get_anchor_pos(const Rectf& rect, AnchorPoint point)
{
  Vector result(0.0f, 0.0f);

  switch (point % 3) {
    case 0: // left
      result.x = rect.get_left();
      break;
    case 1: // middle
      result.x = rect.get_left() + (rect.get_right() - rect.get_left()) / 2.0f;
      break;
    case 2: // right
      result.x = rect.get_right();
      break;
    default:
      log_warning << "Invalid anchor point found" << std::endl;
      result.x = rect.get_left();
      break;
  }

  switch (point / 3) {
    case 0: // top
      result.y = rect.get_top();
      break;
    case 1: // middle
      result.y = rect.get_top() + (rect.get_bottom() - rect.get_top()) / 2.0f;
      break;
    case 2: // bottom
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

  switch (point % 3) {
    case 0: // left
      result.x = destrect.get_left();
      break;
    case 1: // middle
      result.x = destrect.get_middle().x - width / 2.0f;
      break;
    case 2: // right
      result.x = destrect.get_right() - width;
      break;
    default:
      log_warning << "Invalid anchor point found" << std::endl;
      result.x = destrect.get_left();
      break;
  }

  switch (point / 3) {
    case 0: // top
      result.y = destrect.get_top();
      break;
    case 1: // middle
      result.y = destrect.get_middle().y - height / 2.0f;
      break;
    case 2: // bottom
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
