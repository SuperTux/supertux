//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/path_object.hpp"

#include <optional>

#include "editor/editor.hpp"
#include "object/path_gameobject.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

PathObject::PathObject() :
  m_path_handle(),
  m_path_uid(),
  m_walker()
{
}

PathObject::~PathObject()
{
}

void
PathObject::init_path(const ReaderMapping& mapping, bool running_default)
{
  bool running = running_default;
  mapping.get("running", running);

  std::optional<ReaderMapping> handle_map;
  if (mapping.get("handle", handle_map))
  {
    handle_map->get("scale_x", m_path_handle.m_scalar_pos.x);
    handle_map->get("scale_y", m_path_handle.m_scalar_pos.y);
    handle_map->get("offset_x", m_path_handle.m_pixel_offset.x);
    handle_map->get("offset_y", m_path_handle.m_pixel_offset.y);
  }

  std::string path_ref;
  std::optional<ReaderMapping> path_mapping;
  if (mapping.get("path", path_mapping))
  {
    auto& path_gameobject = d_gameobject_manager->add<PathGameObject>(*path_mapping, true);
    m_path_uid = path_gameobject.get_uid();
    m_walker.reset(new PathWalker(m_path_uid, running));
  }
  else if (mapping.get("path-ref", path_ref))
  {
    d_gameobject_manager->request_name_resolve(path_ref, [this, running](UID uid){
        m_path_uid = uid;
        m_walker.reset(new PathWalker(uid, running));
      });
  }
}

void
PathObject::init_path_pos(const Vector& pos, bool running)
{
  auto& path_gameobject = d_gameobject_manager->add<PathGameObject>(pos);
  m_path_uid = path_gameobject.get_uid();
  m_walker.reset(new PathWalker(path_gameobject.get_uid(), running));
}

PathGameObject*
PathObject::get_path_gameobject() const
{
  if(!d_gameobject_manager)
    return nullptr;

  return d_gameobject_manager->get_object_by_uid<PathGameObject>(m_path_uid);
}

Path*
PathObject::get_path() const
{
  auto path_gameobject = get_path_gameobject();
  if(!path_gameobject)
  {
    return nullptr;
  }
  return &path_gameobject->get_path();
}

std::string
PathObject::get_path_ref() const
{
  auto path_gameobject = get_path_gameobject();
  if(!path_gameobject)
  {
    return {};
  }
  return path_gameobject->get_name();
}

void
PathObject::editor_set_path_by_ref(const std::string& new_ref)
{
  auto* path_obj = Editor::current()->get_sector()->get_object_by_name<PathGameObject>(new_ref);
  m_path_uid = path_obj->get_uid();
}

void
PathObject::on_flip()
{
  m_path_handle.m_scalar_pos.y = 1 - m_path_handle.m_scalar_pos.y;
  m_path_handle.m_pixel_offset.y = -m_path_handle.m_pixel_offset.y;
}

/* EOF */
