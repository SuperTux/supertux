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

#include <boost/optional.hpp>

#include "object/path_gameobject.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

PathObject::PathObject() :
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
  if (!d_sector) return;

  bool running = running_default;
  mapping.get("running", running);

  std::string path_ref;
  boost::optional<ReaderMapping> path_mapping;
  if (mapping.get("path", path_mapping))
  {
    auto path_gameobject = d_sector->add<PathGameObject>(*path_mapping);
    m_path_uid = path_gameobject->get_uid();
    m_walker.reset(new PathWalker(m_path_uid, running));
  }
  else if (mapping.get("path-ref", path_ref))
  {
    d_sector->request_name_resolve(path_ref, [this, running](UID uid){
        m_path_uid = uid;
        m_walker.reset(new PathWalker(uid, running));
      });
  }
}

void
PathObject::init_path_pos(const Vector& pos, bool running)
{
  if (!d_sector) return;

  auto path_gameobject = d_sector->add<PathGameObject>(pos);
  m_path_uid = path_gameobject->get_uid();
  m_walker.reset(new PathWalker(path_gameobject->get_uid(), running));
}

void
PathObject::init_path_empty()
{
  if (!d_sector) return;

  auto path_gameobject = d_sector->add<PathGameObject>();
  m_path_uid = path_gameobject->get_uid();
  m_walker.reset(new PathWalker(m_path_uid));
}

Path*
PathObject::get_path()
{
  if (!d_sector) return nullptr;

  auto path_gameobject = d_sector->get_object_by_uid<PathGameObject>(m_path_uid);
  if (!path_gameobject)
  {
    return nullptr;
  }
  else
  {
    return &path_gameobject->get_path();
  }
}

/* EOF */
