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

#include "badguy/snowball.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"

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
  // replace with Snowball
  Vector snowball_pos = get_pos();
  // Hard-coded values from sprites
  snowball_pos.x += 5;
  snowball_pos.y += 1;

  // bounce
  Player* player = dynamic_cast<Player*>(&object);
  if (player)
    player->bounce(*this);

  /* Create death animation for the (now headless) snowman. */
  set_action (dir == LEFT ? "headless-left" : "headless-right", /* loops = */ -1);
  set_pos (get_pos () + Vector (-4.0, 19.0)); /* difference in the sprite offsets */
  physic.set_velocity_y(0);
  physic.set_acceleration_y(0);
  physic.enable_gravity(true);
  set_state (STATE_FALLING);

  /* Create a new snowball where the snowman's head was */
  /* TODO: Pass on our "dead_script" to the snowball. */
  SnowBall* snowball = new SnowBall(snowball_pos, dir);
  Sector::current()->add_object(snowball);

  return true;
}

/* EOF */
