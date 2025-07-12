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
  inline void set_velocity(float nvx, float nvy)
  {
    v = Vector(nvx, nvy);
  }
  void set_velocity(const Vector& vector);

  inline void set_velocity_x(float nvx) { v.x = nvx; }
  inline void set_velocity_y(float nvy) { v.y = nvy; }

  /// Velocity inversion.
  void inverse_velocity_x() { v.x = -v.x; }
  void inverse_velocity_y() { v.y = -v.y; }

  inline Vector get_velocity() const { return v; }
  inline float get_velocity_x() const { return v.x; }
  inline float get_velocity_y() const { return v.y; }

  /// Set acceleration.
  /** Sets acceleration applied to the object. (Note that gravity is
   * eventually added to the vertical acceleration)
   */
  inline void set_acceleration(float nax, float nay)
  {
    a = Vector(nax, nay);
  }
  void set_acceleration(const Vector& vector);

  inline void set_acceleration_x(float nax) { a.x = nax; }
  inline void set_acceleration_y(float nay) { a.y = nay; }

  inline Vector get_acceleration() const { return a; }
  inline float get_acceleration_x() const { return a.x; }
  inline float get_acceleration_y() const { return a.y; }

  /// Enables or disables handling of gravity.
  inline void enable_gravity(bool enable) { gravity_enabled_flag = enable; }
  inline bool gravity_enabled() const { return gravity_enabled_flag; }

  /** Set gravity modifier factor to apply to object when enabled */
  inline void set_gravity_modifier(float modifier) { gravity_modifier = modifier; }

  inline float get_gravity_modifier() const { return gravity_modifier; }

  Vector get_movement(float dt_sec);
  
  // Accelerate the object but prevent overshooting the target velocity
  void accelerate(const Vector& acceleration, float dt_sec, const Vector& target_velocity);
  void accelerate_x(float acceleration, float dt_sec, float target_velocity);
  void accelerate_y(float acceleration, float dt_sec, float target_velocity);

private:
  /** horizontal and vertical acceleration */
  Vector a;

  /** horizontal and vertical velocity */
  Vector v;

  /** should we respect gravity in our calculations? */
  bool gravity_enabled_flag;

  /** gravity modifier is multiplied with the sectors gravity */
  float gravity_modifier;
};

#endif

/* EOF */
