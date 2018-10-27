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
#include "supertux/d_sector.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

void
PathObject::init_path(const ReaderMapping& mapping)
{
  bool running;
  if ( !mapping.get("running", running)) running = false;

  boost::optional<ReaderMapping> path_mapping;
  if (mapping.get("path", path_mapping)) {
    m_path.reset(new Path);
    m_path->read(*path_mapping);
    m_walker.reset(new PathWalker(m_path.get(), running));
  }
}

void
PathObject::init_path_pos(const Vector& pos, bool running)
{
  m_path.reset(new Path(pos));
  m_walker.reset(new PathWalker(m_path.get(), running));
}

void
PathObject::init_path_empty()
{
  m_path.reset(new Path);
  m_walker.reset(new PathWalker(m_path.get()));
}

/* EOF */
