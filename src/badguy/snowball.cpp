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

#include "badguy/snowball.hpp"

#include "sprite/sprite.hpp"

SnowBall::SnowBall(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/snowball/snowball.sprite", "left", "right")
{
  walk_speed = 80;
}

SnowBall::SnowBall(const Vector& pos, Direction d, const std::string& script)
  : WalkingBadguy(pos, d, "images/creatures/snowball/snowball.sprite", "left", "right")
{
  walk_speed = 80;
  m_dead_script = script;
}

bool
SnowBall::collision_squished(GameObject& object)
{
  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

/* EOF */
