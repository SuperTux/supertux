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

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

#include "editor/resize_marker.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "../collision/collision_movement_manager.hpp"

MovingObject::MovingObject() :
  
  m_parent_dispenser(),
  m_group(COLGROUP_MOVING),
  m_bbox(),
  m_movement(0.0f, 0.0f),
  m_dest(),
  m_unisolid(false),
  m_pressure(),
  m_objects_hit_bottom(),
  m_ground_movement_manager(nullptr)
{
}

MovingObject::MovingObject(const ReaderMapping& reader) :
  GameObject(reader),
  m_parent_dispenser(),
  m_group(COLGROUP_MOVING),
  m_bbox(),
  m_movement(0.0f, 0.0f),
  m_dest(),
  m_unisolid(false),
  m_pressure(),
  m_objects_hit_bottom(),
  m_ground_movement_manager(nullptr)
{
  float height, width;

  if (reader.get("width", width))
    m_bbox.set_width(width);

  if (reader.get("height", height))
    m_bbox.set_height(height);

  reader.get("x", m_bbox.get_left());
  reader.get("y", m_bbox.get_top());
}

MovingObject::~MovingObject()
{
}

ObjectSettings
MovingObject::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  if (m_parent_dispenser)
  {
    result.remove("name");
    return result;
  }

  if (has_variable_size())
  {
    result.add_float(_("Width"), &m_bbox.get_width(), "width", {}, OPTION_HIDDEN);
    result.add_float(_("Height"), &m_bbox.get_height(), "height", {}, OPTION_HIDDEN);
  }
  result.add_float(_("X"), &m_bbox.get_left(), "x", {}, OPTION_HIDDEN);
  result.add_float(_("Y"), &m_bbox.get_top(), "y", {}, OPTION_HIDDEN);

  return result;
}

void
MovingObject::set_parent_dispenser(Dispenser* dispenser)
{
  m_parent_dispenser = dispenser;
  if (dispenser)
  {
    m_name.clear();
  }
}

void
MovingObject::editor_select()
{
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::LEFT_UP, ResizeMarker::Side::LEFT_UP);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::LEFT_UP, ResizeMarker::Side::NONE);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::LEFT_UP, ResizeMarker::Side::RIGHT_DOWN);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::NONE, ResizeMarker::Side::LEFT_UP);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::NONE, ResizeMarker::Side::RIGHT_DOWN);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::RIGHT_DOWN, ResizeMarker::Side::LEFT_UP);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::RIGHT_DOWN, ResizeMarker::Side::NONE);
  Sector::get().add<ResizeMarker>(this, ResizeMarker::Side::RIGHT_DOWN, ResizeMarker::Side::RIGHT_DOWN);
}

void
MovingObject::on_flip(float height)
{
  Vector pos = get_pos();
  pos.y = height - pos.y - get_bbox().get_height();
  set_pos(pos);
}

void
MovingObject::collision_moving_object_bottom(MovingObject& other)
{
  if (m_group == COLGROUP_STATIC
    || m_group == COLGROUP_MOVING_STATIC)
  {
    m_objects_hit_bottom.insert(&other);
  }
}

void
MovingObject::notify_object_removal(MovingObject* other)
{
  m_objects_hit_bottom.erase(other);
}

void
MovingObject::clear_bottom_collision_list()
{
  m_objects_hit_bottom.clear();
}

void
MovingObject::propagate_movement(const Vector& movement)
{
  for (MovingObject* other_object : m_objects_hit_bottom) {
    if (other_object->get_group() == COLGROUP_STATIC) continue;
    m_ground_movement_manager->register_movement(*this, *other_object, movement);
    other_object->propagate_movement(movement);
  }
}

void
MovingObject::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addAbstractClass<MovingObject>("MovingObject", vm.findClass("GameObject"));

  cls.addFunc("get_x", &MovingObject::get_x);
  cls.addFunc("get_y", &MovingObject::get_y);
  cls.addFunc<void, MovingObject, float, float>("set_pos", &MovingObject::set_pos);
  cls.addFunc<void, MovingObject, float, float>("move", &MovingObject::move);
  cls.addFunc("get_width", &MovingObject::get_width);
  cls.addFunc("get_height", &MovingObject::get_height);
}

/* EOF */
