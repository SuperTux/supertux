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

#include "badguy/walkingleaf.hpp"

#include "sprite/sprite.hpp"

WalkingLeaf::WalkingLeaf(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/walkingleaf/walkingleaf.sprite", "left", "right")
{
  walk_speed = 60;
  max_drop_height = 16;
}

bool
WalkingLeaf::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  // Spawn death particles
  spawn_explosion_sprites(3, "images/objects/particles/walkingleaf.sprite");
  kill_squished(object);
  return true;
}

bool
WalkingLeaf::is_freezable() const
{
  return true;
}
/* EOF */
