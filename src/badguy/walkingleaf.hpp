//  SuperTux - Walking Leaf
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_WALKINGLEAF_HPP
#define HEADER_SUPERTUX_BADGUY_WALKINGLEAF_HPP

#include "badguy/walking_badguy.hpp"

/*
 * Easy to kill badguy that does not jump down from it's ledge.
 */
class WalkingLeaf : public WalkingBadguy
{
public:
  WalkingLeaf(const ReaderMapping& reader);
  WalkingLeaf(const Vector& pos, Direction d);

  bool is_freezable() const;
  virtual std::string get_class() const {
    return "walkingleaf";
  }
  std::string get_display_name() const {
    return _("Walking leaf");
  }

protected:
  bool collision_squished(GameObject& object);

};

#endif

/* EOF */
