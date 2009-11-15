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

#ifndef HEADER_SUPERTUX_OBJECT_PATH_WALKER_HPP
#define HEADER_SUPERTUX_OBJECT_PATH_WALKER_HPP

#include "object/path.hpp"

/**
 * A walker that travels along a path
 */
class PathWalker
{
public:
  PathWalker(const Path* path, bool running = true);
  virtual ~PathWalker();

  /**
   * advances the path walker on the path and returns the position delta
   * to the last position
   */
  virtual Vector advance(float elapsed_time);

  /** advance until at given node, then stop */
  void goto_node(int node_no);

  /** start advancing automatically */
  void start_moving();

  /** stop advancing automatically */
  void stop_moving();

  /** returns true if PathWalker is currently moving */
  bool is_moving() {
    return running;
  }
  
  const Path* path;

private:
  void advance_node();
  void goback_node();

  /**
   * set to false to immediately stop advancing
   */
  bool running;

  size_t current_node_nr;
  size_t next_node_nr;

  /**
   * stop advancing automatically when this node is reached
   */
  int stop_at_node_nr;

  /**
   * the position between the current node and the next node as fraction
   * between 0 and 1
   */
  float node_time;
  float node_mult;

  float walking_speed;

private:
  PathWalker& operator=(const PathWalker&);
};

#endif
