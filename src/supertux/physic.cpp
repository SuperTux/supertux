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

#include "supertux/moving_object.hpp"
#include "supertux/sector.hpp"

Physic::Physic(MovingObject& parent) :
  m_parent(parent),
  ax(0), ay(0),
  vx(0), vy(0),
  gravity_enabled_flag(true),
  gravity_modifier(1.0f)
{
}

void
Physic::reset()
{
  ax = ay = vx = vy = 0;
  gravity_enabled_flag = true;
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

void
Physic::set_acceleration(float nax, float nay)
{
  ax = nax;
  ay = nay;
}

void
Physic::set_acceleration(const Vector& vector)
{
  ax = vector.x;
  ay = vector.y;
}

Vector
Physic::get_movement(float dt_sec)
{
  assert(m_parent.get_parent_sector());
  float grav = gravity_enabled_flag ? (m_parent.get_parent_sector()->get_gravity() * gravity_modifier * 100.0f) : 0;

  // Semi-implicit Euler integration
  // with constant acceleration, this will result in a position delta of
  // v t + .5 a t (t+dt_sec) at total time t
  vx += ax * dt_sec;
  vy += (ay + grav) * dt_sec;
  Vector result(vx * dt_sec, vy * dt_sec);

  return result;
}

/* EOF */
