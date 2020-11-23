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
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Physic::Physic() :
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
  float grav = gravity_enabled_flag ? (Sector::get().get_gravity() * gravity_modifier * 100.0f) : 0;

  // Semi-implicit Euler integration
  // with constant acceleration, this will result in a position delta of
  // v t + .5 a t (t+dt_sec) at total time t
  vx += ax * dt_sec;
  vy += (ay + grav) * dt_sec;
  Vector result(vx * dt_sec, vy * dt_sec);

  return result;
}

void
Physic::backup(Writer& writer)
{
  writer.write("ax", ax);
  writer.write("ay", ay);
  writer.write("vx", vx);
  writer.write("vy", vy);
  writer.write("gravity_enabled_flag", gravity_enabled_flag);
  writer.write("gravity_modifier", gravity_modifier);
}

void
Physic::restore(const ReaderMapping& reader)
{
  reader.get("ax", ax);
  reader.get("ay", ay);
  reader.get("vx", vx);
  reader.get("vy", vy);
  reader.get("gravity_enabled_flag", gravity_enabled_flag);
  reader.get("gravity_modifier", gravity_modifier);
}

/* EOF */
