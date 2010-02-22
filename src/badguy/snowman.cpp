//  SuperTux
//  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include "badguy/snowman.hpp"

Snowman::Snowman(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/snowman/snowman.sprite", "walk-left", "walk-right")
{
  walk_speed = 40;
}

Snowman::Snowman(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/snowman/snowman.sprite", "walk-left", "walk-right")
{
  walk_speed = 40;
}

bool
Snowman::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

/* EOF */
