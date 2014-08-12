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
  ax(0), ay(0),
  vx(0), vy(0),
  gravity_enabled_flag(true),
  gravity_modifier(1.0f)
{
}

Physic::~Physic()
{
}

void
Physic::reset()
{
  ax = ay = vx = vy = 0;
  gravity_enabled_flag = true;
}

void
Physic::set_velocity_x(float nvx)
{
  vx = nvx;
}

void
Physic::set_velocity_y(float nvy)
{
  vy = nvy;
}

void
Physic::set_velocity(float nvx, float nvy)
{
  vx = nvx;
  vy = nvy;
}

void
Physic::set_velocity(const Vector& vector)
{
  vx = vector.x;
  vy = vector.y;
}

void Physic::inverse_velocity_x()
{
  vx = -vx;
}

void Physic::inverse_velocity_y()
{
  vy = -vy;
}

float
Physic::get_velocity_x() const
{
  return vx;
}

float
Physic::get_velocity_y() const
{
  return vy;
}

Vector
Physic::get_velocity() const
{
  return Vector(vx, vy);
}

void
Physic::set_acceleration_x(float nax)
{
  ax = nax;
}

void
Physic::set_acceleration_y(float nay)
{
  ay = nay;
}

void
Physic::set_acceleration(float nax, float nay)
{
  ax = nax;
  ay = nay;
}

float
Physic::get_acceleration_x() const
{
  return ax;
}

float
Physic::get_acceleration_y() const
{
  return ay;
}

Vector
Physic::get_acceleration() const
{
  return Vector(ax, ay);
}

void
Physic::enable_gravity(bool enable_gravity)
{
  gravity_enabled_flag = enable_gravity;
}

bool
Physic::gravity_enabled() const
{
  return gravity_enabled_flag;
}

void
Physic::set_gravity_modifier(float gravity_modifier)
{
  this->gravity_modifier = gravity_modifier;
}

Vector
Physic::get_movement(float elapsed_time)
{
  float grav = gravity_enabled_flag ? (Sector::current()->get_gravity() * gravity_modifier * 100.0f) : 0;

  // Semi-implicit Euler integration
  // with constant acceleration, this will result in a position delta of
  // v t + .5 a t (t+elapsed_time) at total time t
  vx += ax * elapsed_time;
  vy += (ay + grav) * elapsed_time;
  Vector result(vx * elapsed_time, vy * elapsed_time);

  return result;
}

/* EOF */
