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
#include "util/log.hpp"
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
MovingObject::backup(Writer& writer)
{
  GameObject::backup(writer);

  writer.start_list(MovingObject::get_class());
  writer.write("x1", m_col.m_bbox.get_left());
  writer.write("y1", m_col.m_bbox.get_top());
  writer.write("x2", m_col.m_bbox.get_right());
  writer.write("y2", m_col.m_bbox.get_bottom());
  writer.write("movement_x", m_col.m_movement.x);
  writer.write("movement_y", m_col.m_movement.y);
  writer.write("dest_x1", m_col.m_dest.get_left());
  writer.write("dest_y1", m_col.m_dest.get_top());
  writer.write("dest_x2", m_col.m_dest.get_right());
  writer.write("dest_y2", m_col.m_dest.get_bottom());
  writer.write("group", static_cast<int>(m_col.m_group));
  writer.end_list(MovingObject::get_class());
}

void
MovingObject::restore(const ReaderMapping& reader)
{
  GameObject::restore(reader);

  boost::optional<ReaderMapping> subreader(ReaderMapping(reader.get_doc(), reader.get_sexp()));

  if (reader.get(MovingObject::get_class().c_str(), subreader))
  {
    float x1;
    if (subreader->get("x1", x1))
      m_col.m_bbox.set_left(x1);

    float y1;
    if (subreader->get("y1", y1))
      m_col.m_bbox.set_top(y1);

    float x2;
    if (subreader->get("x2", x2))
      m_col.m_bbox.set_right(x2);

    float y2;
    if (subreader->get("y2", y2))
      m_col.m_bbox.set_bottom(y2);

    float dest_x1;
    if (subreader->get("dest_x1", dest_x1))
      m_col.m_dest.set_left(dest_x1);

    float dest_y1;
    if (subreader->get("dest_y1", dest_y1))
      m_col.m_dest.set_top(dest_y1);

    float dest_x2;
    if (subreader->get("dest_x2", dest_x2))
      m_col.m_dest.set_right(dest_x2);

    float dest_y2;
    if (subreader->get("dest_y2", dest_y2))
      m_col.m_dest.set_bottom(dest_y2);

    int group;
    if (subreader->get("group", group))
      m_col.m_group = static_cast<CollisionGroup>(group);

    subreader->get("movement_x", m_col.m_movement.x);
    subreader->get("movement_y", m_col.m_movement.y);
  }
}

/* EOF */
