//  SuperTux - Smart Snowball
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#include "badguy/smartball.hpp"

#include "sprite/sprite.hpp"

SmartBall::SmartBall(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/snowball/smart-snowball.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
}

bool
SmartBall::collision_squished(GameObject& object)
{
  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

/* EOF */
