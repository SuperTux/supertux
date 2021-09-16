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

#include <string.h>
#include <memory>

#include "object/path.hpp"
#include "util/uid.hpp"

class ObjectOption;

/** A walker that travels along a path */
class PathWalker final
{
public:
  PathWalker(UID path_uid, bool running = true);
  ~PathWalker();

  /** advances the path walker on the path and returns its new position */
  void update(float dt_sec);

  /** current position of path walker */
  Vector get_pos() const;

  /** advance until at given node, then stop */
  void goto_node(int node_no);

  /** start advancing automatically */
  void start_moving();

  /** stop advancing automatically */
  void stop_moving();

  /** returns true if PathWalker is currently moving */
  bool is_running() const { return m_running; }

  void backup(Writer& writer) const;
  void restore(const ReaderMapping& reader);

private:
  void advance_node();
  void goback_node();
  Path* get_path() const;

public:
  UID m_path_uid;

  /** set to false to immediately stop advancing */
  bool m_running;

private:
  size_t m_current_node_nr;
  size_t m_next_node_nr;

  /** stop advancing automatically when this node is reached */
  int m_stop_at_node_nr;

  /** the position between the current node and the next node as
      fraction between 0 and 1 */
  float m_node_time;
  float m_node_mult;

  float m_walking_speed;

private:
  PathWalker(const PathWalker&) = delete;
  PathWalker& operator=(const PathWalker&) = delete;
};

#endif

/* EOF */
