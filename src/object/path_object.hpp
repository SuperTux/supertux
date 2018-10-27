//  SuperTux
//  Copyright (C) 2018 Tobias Markus <tobbi@supertux.org>
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


#ifndef HEADER_SUPERTUX_OBJECT_PATH_OBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_PATH_OBJECT_HPP

#include <memory>

#include "object/path.hpp"
#include "object/path_walker.hpp"

/** A class for all objects that contain / make use of a path. */
class PathObject
{
public:
  PathObject() :
    m_path(),
    m_walker()
  {
  }

  virtual ~PathObject()
  {
  }

  /** For compatibilty reasons this needs to get the GameObjects
      main mapping, not the (path ...) mapping */
  void init_path(const ReaderMapping& mapping);
  void init_path_pos(const Vector& pos, bool running = false);
  void init_path_empty();

  /** Returns this object's path */
  Path* get_path() const {
    return m_path.get();
  }

  /** Returns this object's path walker */
  PathWalker* get_walker() const {
    return m_walker.get();
  }

private:
  std::unique_ptr<Path> m_path;
  std::unique_ptr<PathWalker> m_walker;

private:
  PathObject(const PathObject&) = delete;
  PathObject& operator=(const PathObject&) = delete;
};

#endif

/* EOF */
