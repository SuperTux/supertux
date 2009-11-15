//  SuperTux - WalkingBadguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/walking_badguy.hpp"

#include "lisp/writer.hpp"
#include "sprite/sprite.hpp"

WalkingBadguy::WalkingBadguy(const Vector& pos, 
                             const std::string& sprite_name, 
                             const std::string& walk_left_action, 
                             const std::string& walk_right_action, 
                             int layer) :
  BadGuy(pos, sprite_name, layer), 
  walk_left_action(walk_left_action), 
  walk_right_action(walk_right_action), 
  walk_speed(80), 
  max_drop_height(-1),
  turn_around_timer(),
  turn_around_counter()
{
}

WalkingBadguy::WalkingBadguy(const Vector& pos, 
                             Direction direction, 
                             const std::string& sprite_name, 
                             const std::string& walk_left_action, 
                             const std::string& walk_right_action, 
                             int layer) :
  BadGuy(pos, direction, sprite_name, layer), 
  walk_left_action(walk_left_action), 
  walk_right_action(walk_right_action), 
  walk_speed(80), 
  max_drop_height(-1),
  turn_around_timer(),
  turn_around_counter()
{
}

WalkingBadguy::WalkingBadguy(const lisp::Lisp& reader, 
                             const std::string& sprite_name, 
                             const std::string& walk_left_action, 
                             const std::string& walk_right_action, 
                             int layer) :
  BadGuy(reader, sprite_name, layer), 
  walk_left_action(walk_left_action), 
  walk_right_action(walk_right_action), 
  walk_speed(80), 
  max_drop_height(-1),
  turn_around_timer(),
  turn_around_counter()
{
}

void
WalkingBadguy::write(lisp::Writer& writer)
{
  writer.write("x", start_position.x);
  writer.write("y", start_position.y);
}

void
WalkingBadguy::initialize()
{
  if(frozen)
    return;
  sprite->set_action(dir == LEFT ? walk_left_action : walk_right_action);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  physic.set_velocity_x(dir == LEFT ? -walk_speed : walk_speed);
}

void
WalkingBadguy::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  if (max_drop_height > -1) {
    if (on_ground() && might_fall(max_drop_height+1))
    {
      turn_around();
    }
  }

}

void
WalkingBadguy::collision_solid(const CollisionHit& hit)
{

  update_on_ground_flag(hit);

  if (hit.top) {
    if (physic.get_velocity_y() < 0) physic.set_velocity_y(0);
  }
  if (hit.bottom) {
    if (physic.get_velocity_y() > 0) physic.set_velocity_y(0);
  }

  if ((hit.left && (hit.slope_normal.y == 0) && (dir == LEFT)) || (hit.right && (hit.slope_normal.y == 0) && (dir == RIGHT))) {
    turn_around();
  }

}

HitResponse
WalkingBadguy::collision_badguy(BadGuy& , const CollisionHit& hit)
{

  if ((hit.left && (dir == LEFT)) || (hit.right && (dir == RIGHT))) {
    turn_around();
  }

  return CONTINUE;
}

void
WalkingBadguy::turn_around()
{
  if(frozen)
    return;
  dir = dir == LEFT ? RIGHT : LEFT;
  sprite->set_action(dir == LEFT ? walk_left_action : walk_right_action);
  physic.set_velocity_x(-physic.get_velocity_x());

  // if we get dizzy, we fall off the screen
  if (turn_around_timer.started()) {
    if (turn_around_counter++ > 10) kill_fall();
  } else {
    turn_around_timer.start(1);
    turn_around_counter = 0;
  }

}

void
WalkingBadguy::freeze()
{
  BadGuy::freeze();
  physic.set_velocity_x(0);
}

void
WalkingBadguy::unfreeze()
{
  BadGuy::unfreeze();
  WalkingBadguy::initialize();
}

float
WalkingBadguy::get_velocity_y() const
{
  return physic.get_velocity_y();
}

void
WalkingBadguy::set_velocity_y(float vy)
{
  physic.set_velocity_y(vy);
}

/* EOF */
