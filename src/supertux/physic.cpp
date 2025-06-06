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

#include "supertux/physic.hpp"

#include "supertux/sector.hpp"

Physic::Physic() :
  a(0, 0),
  v(0, 0),
  gravity_enabled_flag(true),
  gravity_modifier(1.0f)
{
}

void
Physic::reset()
{
  a.x = a.y = v.x = v.y = 0;
  gravity_enabled_flag = true;
}

void
Physic::set_velocity(const Vector& vector)
{
  v = vector;
}

void
Physic::set_acceleration(const Vector& vector)
{
  a = vector;
}

Vector
Physic::get_movement(float dt_sec)
{
  float grav = gravity_enabled_flag ? (Sector::get().get_gravity() * gravity_modifier * 100.0f) : 0;

  // Semi-implicit Euler integration
  // with constant acceleration, this will result in a position delta of
  // v t + .5 a t (t+dt_sec) at total time t
  v.x += a.x * dt_sec;
  v.y += (a.y + grav) * dt_sec;
  Vector result(v.x * dt_sec, v.y * dt_sec);

  return result;
}

void
Physic::accelerate(const Vector& acceleration, float dt_sec, const Vector& target_velocity)
{
  const Vector v_contr = acceleration * dt_sec;
  const Vector new_v = v + v_contr;
  if (glm::dot(v_contr, target_velocity - new_v) < 0) {
    v = target_velocity; // overshot detected, trimming velocity
  } else {
    v = new_v;
  }
}

void
Physic::accelerate_x(float acceleration, float dt_sec, float target_velocity)
{
  const float v_contr = acceleration * dt_sec;
  const float new_v = v.x + v_contr;
  if (v_contr * (target_velocity - new_v) < 0) {
    v.x = target_velocity;
  } else {
    v.x = new_v;
  }
}

void
Physic::accelerate_y(float acceleration, float dt_sec, float target_velocity)
{
  const float v_contr = acceleration * dt_sec;
  const float new_v = v.y + v_contr;
  if (v_contr * (target_velocity - new_v) < 0) {
    v.y = target_velocity;
  } else {
    v.y = new_v;
  }
}

/* EOF */
