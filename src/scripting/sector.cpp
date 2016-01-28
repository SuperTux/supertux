//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/sector.hpp"

#include <physfs.h>

#include "supertux/sector.hpp"

namespace scripting {

Sector::Sector(::Sector* parent) :
  m_parent(parent)
{
}

Sector::~Sector()
{
}

void
Sector::set_ambient_light(float red, float green, float blue)
{
  m_parent->set_ambient_light(red, green, blue);
}

float
Sector::get_ambient_red() const
{
  return m_parent->get_ambient_red();
}

float
Sector::get_ambient_green() const
{
  return m_parent->get_ambient_green();
}

float
Sector::get_ambient_blue() const
{
  return m_parent->get_ambient_blue();
}

void
Sector::set_gravity(float gravity)
{
  m_parent->set_gravity(gravity);
}

} // namespace scripting

/* EOF */
