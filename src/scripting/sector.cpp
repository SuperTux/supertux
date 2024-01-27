//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
//                2021 A. Semphris <semphris@protonmail.com>
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

#include "supertux/sector.hpp"

namespace scripting {

Sector::Sector(::Sector* parent) :
  GameObjectManager(parent),
  m_parent(parent)
{
}

void
Sector::set_gravity(float gravity)
{
  m_parent->set_gravity(gravity);
}

bool
Sector::is_free_of_solid_tiles(float left, float top, float right, float bottom,
                               bool ignore_unisolid) const
{
  return m_parent->is_free_of_tiles(Rectf(Vector(left, top), Vector(right, bottom)),
                                    ignore_unisolid, Tile::SOLID);
}

bool
Sector::is_free_of_statics(float left, float top, float right, float bottom,
                           bool ignore_unisolid) const
{
  return m_parent->is_free_of_statics(Rectf(Vector(left, top), Vector(right, bottom)),
                                      nullptr, ignore_unisolid);
}

bool
Sector::is_free_of_movingstatics(float left, float top, float right, float bottom) const
{
  return m_parent->is_free_of_movingstatics(Rectf(Vector(left, top), Vector(right, bottom)));
}

bool
Sector::is_free_of_specifically_movingstatics(float left, float top, float right, float bottom) const
{
  return m_parent->is_free_of_specifically_movingstatics(Rectf(Vector(left, top), Vector(right, bottom)));
}

} // namespace scripting

/* EOF */
