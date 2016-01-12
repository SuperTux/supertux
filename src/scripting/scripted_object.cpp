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

ScriptedObject::ScriptedObject(::ScriptedObject* parent) :
  m_parent(parent)
{
}

ScriptedObject::~ScriptedObject()
{
}

void
ScriptedObject::set_action(const std::string& animation)
{
  m_parent->set_action(animation);
}

std::string
ScriptedObject::get_action()
{
  return m_parent->get_action();
}

void
ScriptedObject::move(float x, float y)
{
  m_parent->move(x, y);
}

void
ScriptedObject::set_pos(float x, float y)
{
  m_parent->set_pos(Vector(x, y));
}

float
ScriptedObject::get_pos_x()
{
  return m_parent->get_pos_x();
}

float
ScriptedObject::get_pos_y()
{
  return m_parent->get_pos_y();
}

void
ScriptedObject::set_velocity(float x, float y)
{
  m_parent->set_velocity(x, y);
}

float
ScriptedObject::get_velocity_x()
{
  return m_parent->get_velocity_x();
}

float
ScriptedObject::get_velocity_y()
{
  return m_parent->get_velocity_y();
}

void
ScriptedObject::enable_gravity(bool f)
{
  m_parent->enable_gravity(f);
}

bool
ScriptedObject::gravity_enabled() const
{
  return m_parent->gravity_enabled();
}

void
ScriptedObject::set_visible(bool visible)
{
  m_parent->set_visible(visible);
}

bool
ScriptedObject::is_visible()
{
  return m_parent->is_visible();
}

void
ScriptedObject::set_solid(bool solid)
{
  return m_parent->set_solid(solid);
}

bool
ScriptedObject::is_solid()
{
  return m_parent->is_solid();
}

std::string
ScriptedObject::get_name()
{
  return m_parent->get_name();
}

} // namespace scripting

/* EOF */
