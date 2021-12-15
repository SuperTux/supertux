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
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

MovingObject::MovingObject() :
  m_col(COLGROUP_MOVING, *this)
{
}

MovingObject::MovingObject(const ReaderMapping& reader) :
  GameObject(reader),
  m_col(COLGROUP_MOVING, *this)
{
  float height, width;

  if (reader.get("width", width))
    m_col.m_bbox.set_width(width);

  if (reader.get("height", height))
    m_col.m_bbox.set_height(height);

  reader.get("x", m_col.m_bbox.get_left());
  reader.get("y", m_col.m_bbox.get_top());
}

MovingObject::~MovingObject()
{
}

ObjectSettings
MovingObject::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  if (has_variable_size()) {
    result.add_rectf(_("Region"), &m_col.m_bbox, "region", OPTION_HIDDEN);
  }

  result.add_float(_("X"), &m_col.m_bbox.get_left(), "x", {}, OPTION_HIDDEN);
  result.add_float(_("Y"), &m_col.m_bbox.get_top(), "y", {}, OPTION_HIDDEN);

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

void
MovingObject::on_flip(float height)
{
  Vector pos = get_pos();
  pos.y = height - pos.y - get_bbox().get_height();
  set_pos(pos);
}

/* EOF */
