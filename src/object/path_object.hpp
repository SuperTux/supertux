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

/**
 * A class for all objects that contain / make use of a path.
 */
class PathObject
{
public:
  std::shared_ptr<Path> path;
  std::shared_ptr<PathWalker> walker;

  PathObject() :
    path(),
    walker()
  {
  }

  PathObject(const PathObject& other) :
    path(other.path),
    walker(other.walker)
  {
  }

  virtual ~PathObject()
  {
  }
  /**
   * Returns this object's path
   */
  Path* get_path() const {
    return path.get();
  }

  /**
   * Returns this object's path walker
   */
  PathWalker* get_walker() const {
    return walker.get();
  }
};

#endif