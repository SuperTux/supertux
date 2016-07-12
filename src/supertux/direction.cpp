//  SuperTux
//  Copyright (C) 2008 Ryan Flegel <rflegel@gmail.com>
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

#include <vector>

#include "supertux/direction.hpp"

#include "editor/object_option.hpp"
#include "util/gettext.hpp"

std::ostream& operator<<(std::ostream& o, const Direction& dir)
{
  switch (dir)
  {
    case AUTO:
      o << "auto";
      break;
    case LEFT:
      o << "left";
      break;
    case RIGHT:
      o << "right";
      break;
    case UP:
      o << "up";
      break;
    case DOWN:
      o << "down";
      break;
  }

  return o;
}

ObjectOption
dir_option(Direction *dir) {
  ObjectOption result(MN_STRINGSELECT, _("Direction"), dir);
  result.select.push_back(_("auto"));
  result.select.push_back(_("left"));
  result.select.push_back(_("right"));
  result.select.push_back(_("up"));
  result.select.push_back(_("down"));
  return result;
}

std::string dir_to_string(const Direction& dir) {
  switch(dir)
  {
    case LEFT:
      return "left";
    case RIGHT:
      return "right";
    case UP:
      return "up";
    case DOWN:
      return "down";
    case AUTO:
    default:
      return "auto";
  }
}

std::string dir_to_prefixed(const Direction& dir, const std::string& prefix) {
  return prefix + "-" + dir_to_string(dir);
}

Direction string_to_dir(const std::string& dir_str) {
  if(dir_str == "left")
    return LEFT;
  else if(dir_str == "right")
    return RIGHT;
  else if(dir_str == "up")
    return UP;
  else if(dir_str == "down")
    return DOWN;
  else
    return AUTO;
}

/* EOF */
