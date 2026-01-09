//  SuperTux - Walking Leaf
//  Copyright (C) 2026 MatusGuy
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

#include "badguy/walkingleaf.hpp"

WalkingLeaf::WalkingLeaf(const ReaderMapping& reader) :
  ViciousIvy(reader, "images/creatures/walkingleaf/walkingleaf.sprite")
{
  parse_type(reader);

  set_ledge_behavior(LedgeBehavior::SMART);
}

std::string
WalkingLeaf::get_default_sprite_name() const
{
  switch (m_type)
  {
    case CORRUPTED:
      return "images/creatures/walkingleaf/corrupted/rotten_leaf.sprite";
    default:
      return m_default_sprite_name;
  }
}

void
WalkingLeaf::on_type_change(int old_type)
{
  MovingSprite::on_type_change(old_type);

  switch (m_type)
  {
    case NORMAL:
      walk_speed = 60.f;
      m_fall_speed = 35.f;
      break;
    case CORRUPTED:
      walk_speed = 55.f;
      m_fall_speed = 80.f;
      break;
    default:
      break;
  }
}

std::string
WalkingLeaf::get_explosion_sprite() const
{
  switch (m_type)
  {
    case CORRUPTED:
      return "images/particles/rottenleaf.sprite";
    default:
      return "images/particles/walkingleaf.sprite";
  }
}

/* EOF */
