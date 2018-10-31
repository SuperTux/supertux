//  SuperTux
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

#include "supertux/moving_object.hpp"

#include "editor/resizer.hpp"
#include "supertux/sector.hpp"
#include "util/writer.hpp"

MovingObject::MovingObject() :
  m_col(COLGROUP_MOVING, *this)
{
}

MovingObject::MovingObject(const ReaderMapping& reader) :
  GameObject(reader),
  m_col(COLGROUP_MOVING, *this)
{
}

MovingObject::~MovingObject()
{
}

void
MovingObject::save(Writer& writer)
{
  GameObject::save(writer);
  writer.write("x", m_col.m_bbox.p1.x);
  writer.write("y", m_col.m_bbox.p1.y);
}

void
MovingObject::edit_bbox() {
  if (!is_valid()) {
    return;
  }

  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::LEFT_UP, Resizer::LEFT_UP);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::LEFT_UP, Resizer::NONE);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::LEFT_UP, Resizer::RIGHT_DOWN);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::NONE, Resizer::LEFT_UP);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::NONE, Resizer::RIGHT_DOWN);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::RIGHT_DOWN, Resizer::LEFT_UP);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::RIGHT_DOWN, Resizer::NONE);
  Sector::get().add<Resizer>(&m_col.m_bbox, Resizer::RIGHT_DOWN, Resizer::RIGHT_DOWN);
}

/* EOF */
