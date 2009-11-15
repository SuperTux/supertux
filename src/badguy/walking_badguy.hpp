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

#ifndef HEADER_SUPERTUX_BADGUY_WALKING_BADGUY_HPP
#define HEADER_SUPERTUX_BADGUY_WALKING_BADGUY_HPP

#include "badguy.hpp"

class Timer;

/**
 * Base class for Badguys that walk on the floor.
 */
class WalkingBadguy : public BadGuy
{
public:
  WalkingBadguy(const Vector& pos, const std::string& sprite_name, const std::string& walk_left_action, const std::string& walk_right_action, int layer = LAYER_OBJECTS);
  WalkingBadguy(const Vector& pos, Direction direction, const std::string& sprite_name, const std::string& walk_left_action, const std::string& walk_right_action, int layer = LAYER_OBJECTS);
  WalkingBadguy(const lisp::Lisp& reader, const std::string& sprite_name, const std::string& walk_left_action, const std::string& walk_right_action, int layer = LAYER_OBJECTS);

  void initialize();
  void write(lisp::Writer& writer);
  void active_update(float elapsed_time);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  void freeze();
  void unfreeze();

  float get_velocity_y() const;
  void set_velocity_y(float vy);

protected:
  void turn_around();

  std::string walk_left_action;
  std::string walk_right_action;
  float walk_speed;
  int max_drop_height; /**< Maximum height of drop before we will turn around, or -1 to just drop from any ledge */
  Timer turn_around_timer;
  int turn_around_counter; /**< counts number of turns since turn_around_timer was started */
};

#endif
