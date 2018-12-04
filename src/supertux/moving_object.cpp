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

#include "editor/resize_marker.hpp"
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

#if 0
void
MovingObject::save(Writer& writer)
{
  GameObject::save(writer);
  writer.write("x", m_col.m_bbox.p1.x);
  writer.write("y", m_col.m_bbox.p1.y);
}
#endif

ObjectSettings
MovingObject::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  result.add_float(_("X"), &m_col.m_bbox.p1.x, "x");
  result.add_float(_("Y"), &m_col.m_bbox.p1.y, "y");

  if (has_variable_size()) {
    /*
    result.add_float(_("Width"), , "width");
    result.add_float(_("Height"), &m_col.m_bbox.p1.x, "height");
    */
  }

  return result;
}

void
MovingObject::editor_select()
{
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::LEFT_UP, ResizeMarker::Side::LEFT_UP);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::LEFT_UP, ResizeMarker::Side::NONE);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::LEFT_UP, ResizeMarker::Side::RIGHT_DOWN);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::NONE, ResizeMarker::Side::LEFT_UP);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::NONE, ResizeMarker::Side::RIGHT_DOWN);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::RIGHT_DOWN, ResizeMarker::Side::LEFT_UP);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::RIGHT_DOWN, ResizeMarker::Side::NONE);
  Sector::get().add<ResizeMarker>(&m_col.m_bbox, ResizeMarker::Side::RIGHT_DOWN, ResizeMarker::Side::RIGHT_DOWN);
}

/* EOF */
