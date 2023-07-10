//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/scripted_object.hpp"

#include "object/scripted_object.hpp"

namespace scripting {

void
ScriptedObject::set_action(const std::string& animation)
{
  SCRIPT_GUARD_VOID;
  object.set_action(animation);
}

std::string
ScriptedObject::get_action() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_action();
}

std::string
ScriptedObject::get_name() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_name();
}

void
ScriptedObject::move(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.move(x, y);
}

void
ScriptedObject::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_pos(Vector(x, y));
}

float
ScriptedObject::get_pos_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos_x();
}

float
ScriptedObject::get_pos_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos_y();
}

void
ScriptedObject::set_velocity(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_velocity(x, y);
}

float
ScriptedObject::get_velocity_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_velocity_x();
}

float
ScriptedObject::get_velocity_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_velocity_y();
}

void
ScriptedObject::enable_gravity(bool enabled)
{
  SCRIPT_GUARD_VOID;
  object.enable_gravity(enabled);
}

bool
ScriptedObject::gravity_enabled() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.gravity_enabled();
}

void
ScriptedObject::set_visible(bool visible)
{
  SCRIPT_GUARD_VOID;
  object.set_visible(visible);
}

bool
ScriptedObject::is_visible() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.is_visible();
}

void
ScriptedObject::set_solid(bool solid)
{
  SCRIPT_GUARD_VOID;
  return object.set_solid(solid);
}

bool
ScriptedObject::is_solid() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.is_solid();
}

} // namespace scripting

/* EOF */
