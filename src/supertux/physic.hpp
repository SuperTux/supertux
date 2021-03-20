//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_PHYSIC_HPP
#define HEADER_SUPERTUX_SUPERTUX_PHYSIC_HPP

#include "math/vector.hpp"

/// Physics engine.
/** This is a very simplistic physics engine handling accelerated and constant
 * movement along with gravity.
 */
class Physic final
{
public:
  Physic();

  /// Resets all velocities and accelerations to 0.
  void reset();

  /// Sets velocity to a fixed value.
  void set_velocity(float nvx, float nvy);
  void set_velocity(const Vector& vector);

  void set_velocity_x(float nvx) { vx = nvx; }
  void set_velocity_y(float nvy) { vy = nvy; }

  /// Velocity inversion.
  void inverse_velocity_x() { vx = -vx; }
  void inverse_velocity_y() { vy = -vy; }

  Vector get_velocity() const { return Vector(vx, vy); }
  float get_velocity_x() const { return vx; }
  float get_velocity_y() const { return vy; }

  /// Set acceleration.
  /** Sets acceleration applied to the object. (Note that gravity is
   * eventually added to the vertical acceleration)
   */
  void set_acceleration(float nax, float nay);
  void set_acceleration(const Vector& vector);

  void set_acceleration_x(float nax) { ax = nax; }
  void set_acceleration_y(float nay) { ay = nay; }

  Vector get_acceleration() const { return Vector(ax, ay); }
  float get_acceleration_x() const { return ax; }
  float get_acceleration_y() const { return ay; }

  /// Enables or disables handling of gravity.
  void enable_gravity(bool enable) { gravity_enabled_flag = enable; }
  bool gravity_enabled() const { return gravity_enabled_flag; }

  /** Set gravity modifier factor to apply to object when enabled */
  void set_gravity_modifier(float modifier) { gravity_modifier = modifier; }

  Vector get_movement(float dt_sec);

private:
  /** horizontal and vertical acceleration */
  float ax, ay;

  /** horizontal and vertical velocity */
  float vx, vy;

  /** should we respect gravity in our calculations? */
  bool gravity_enabled_flag;

  /** gravity modifier is multiplied with the sectors gravity */
  float gravity_modifier;
};

#endif

/* EOF */
