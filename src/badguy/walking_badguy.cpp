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

#include <math.h>

#include "sprite/sprite.hpp"

WalkingBadguy::WalkingBadguy(const Vector& pos,
                             const std::string& sprite_name_,
                             const std::string& walk_left_action_,
                             const std::string& walk_right_action_,
                             int layer_,
                             const std::string& light_sprite_name) :
  BadGuy(pos, sprite_name_, layer_, light_sprite_name),
  walk_left_action(walk_left_action_),
  walk_right_action(walk_right_action_),
  walk_speed(80),
  max_drop_height(-1),
  turn_around_timer(),
  turn_around_counter()
{
}

WalkingBadguy::WalkingBadguy(const Vector& pos,
                             Direction direction,
                             const std::string& sprite_name_,
                             const std::string& walk_left_action_,
                             const std::string& walk_right_action_,
                             int layer_,
                             const std::string& light_sprite_name) :
  BadGuy(pos, direction, sprite_name_, layer_, light_sprite_name),
  walk_left_action(walk_left_action_),
  walk_right_action(walk_right_action_),
  walk_speed(80),
  max_drop_height(-1),
  turn_around_timer(),
  turn_around_counter()
{
}

WalkingBadguy::WalkingBadguy(const ReaderMapping& reader,
                             const std::string& sprite_name_,
                             const std::string& walk_left_action_,
                             const std::string& walk_right_action_,
                             int layer_,
                             const std::string& light_sprite_name) :
  BadGuy(reader, sprite_name_, layer_, light_sprite_name),
  walk_left_action(walk_left_action_),
  walk_right_action(walk_right_action_),
  walk_speed(80),
  max_drop_height(-1),
  turn_around_timer(),
  turn_around_counter()
{
}

void
WalkingBadguy::initialize()
{
  if(frozen)
    return;
  sprite->set_action(dir == LEFT ? walk_left_action : walk_right_action);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  physic.set_velocity_x(dir == LEFT ? -walk_speed : walk_speed);
  physic.set_acceleration_x (0.0);
}

void
WalkingBadguy::set_walk_speed (float ws)
{
  walk_speed = fabs (ws);
  /* physic.set_velocity_x(dir == LEFT ? -walk_speed : walk_speed); */
}

void
WalkingBadguy::add_velocity (const Vector& velocity)
{
  physic.set_velocity(physic.get_velocity() + velocity);
}

void
WalkingBadguy::active_update(float elapsed_time, float dest_x_velocity)
{
  BadGuy::active_update(elapsed_time);

  float current_x_velocity = physic.get_velocity_x ();

  if (frozen)
  {
    physic.set_velocity_x (0.0);
    physic.set_acceleration_x (0.0);
  }
  /* We're very close to our target speed. Just set it to avoid oscillation */
  else if ((current_x_velocity > (dest_x_velocity - 5.0))
      && (current_x_velocity < (dest_x_velocity + 5.0)))
  {
    physic.set_velocity_x (dest_x_velocity);
    physic.set_acceleration_x (0.0);
  }
  /* Check if we're going too slow or even in the wrong direction */
  else if (((dest_x_velocity <= 0.0) && (current_x_velocity > dest_x_velocity))
      || ((dest_x_velocity > 0.0) && (current_x_velocity < dest_x_velocity)))
  {
    /* acceleration == walk-speed => it will take one second to get from zero
     * to full speed. */
    physic.set_acceleration_x (dest_x_velocity);
  }
  /* Check if we're going too fast */
  else if (((dest_x_velocity <= 0.0) && (current_x_velocity < dest_x_velocity))
      || ((dest_x_velocity > 0.0) && (current_x_velocity > dest_x_velocity)))
  {
    /* acceleration == walk-speed => it will take one second to get twice the
     * speed to normal speed. */
    physic.set_acceleration_x ((-1.f) * dest_x_velocity);
  }
  else
  {
    /* The above should have covered all cases. */
    assert (23 == 42);
  }

  if (max_drop_height > -1) {
    if (on_ground() && might_fall(max_drop_height+1))
    {
      turn_around();
    }
  }

  if ((dir == LEFT) && (physic.get_velocity_x () > 0.0)) {
    dir = RIGHT;
    set_action (walk_right_action, /* loops = */ -1);
  }
  else if ((dir == RIGHT) && (physic.get_velocity_x () < 0.0)) {
    dir = LEFT;
    set_action (walk_left_action, /* loops = */ -1);
  }
}

void
WalkingBadguy::active_update(float elapsed_time)
{
  active_update (elapsed_time, (dir == LEFT) ? -walk_speed : +walk_speed);
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

  if ((hit.left && (dir == LEFT)) || (hit.right && (dir == RIGHT))) {
    turn_around();
  }

}

HitResponse
WalkingBadguy::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(hit.top) {
    return FORCE_MOVE;
  }

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
  if (get_state() == STATE_INIT || get_state() == STATE_INACTIVE || get_state() == STATE_ACTIVE) {
    sprite->set_action(dir == LEFT ? walk_left_action : walk_right_action);
  }
  physic.set_velocity_x(-physic.get_velocity_x());
  physic.set_acceleration_x (-physic.get_acceleration_x ());

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

void
WalkingBadguy::after_editor_set()
{
  BadGuy::after_editor_set();
  sprite->set_action(dir == LEFT ? walk_left_action : walk_right_action);
}

/* EOF */
