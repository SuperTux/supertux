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
#ifndef SUPERTUX_PHYSIC_H
#define SUPERTUX_PHYSIC_H

#include "math/vector.hpp"

/// Physics engine.
/** This is a very simplistic physics engine handling accelerated and constant
  * movement along with gravity.
  */
class Physic
{
public:
  Physic();
  ~Physic();

  /// Resets all velocities and accelerations to 0.
  void reset();

  /// Sets velocity to a fixed value.
  void set_velocity(float vx, float vy);
  void set_velocity(const Vector& vector);

  void set_velocity_x(float vx);
  void set_velocity_y(float vy);

  /// Velocity inversion.
  void inverse_velocity_x();
  void inverse_velocity_y();

  Vector get_velocity() const;
  float get_velocity_x() const;
  float get_velocity_y() const;

  /// Set acceleration.
  /** Sets acceleration applied to the object. (Note that gravity is
   * eventually added to the vertical acceleration)
   */
  void set_acceleration(float ax, float ay);

  void set_acceleration_x(float ax);
  void set_acceleration_y(float ay);

  Vector get_acceleration() const;
  float get_acceleration_x() const;
  float get_acceleration_y() const;

  /// Enables or disables handling of gravity.
  void enable_gravity(bool gravity_enabled);
  bool gravity_enabled() const;

  /// Set gravity to apply to object when enabled
  void set_gravity(float gravity);

  /// Get gravity to apply to object when enabled
  float get_gravity() const;

  Vector get_movement(float elapsed_time);

private:
  /// horizontal and vertical acceleration
  float ax, ay;
  /// horizontal and vertical velocity
  float vx, vy;
  /// should we respect gravity in our calculations?
  bool gravity_enabled_flag;
  /// current gravity (multiplied by 100) to apply to object, if enabled
  float gravity;
};

class UsesPhysic
{
public:
  UsesPhysic() : 
    physic() 
  {}
  virtual ~UsesPhysic() {}

  Physic physic;
  friend class Sector;
};

#endif
