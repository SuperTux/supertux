//  SuperTux Path
//  Copyright (C) 2005 Philipp <balinor@pnxs.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_PATH_HPP
#define HEADER_SUPERTUX_OBJECT_PATH_HPP

#include <memory>
#include <string>
#include <vector>

#include "math/vector.hpp"
#include "math/easing.hpp"

class ObjectOption;
class ReaderMapping;
class Writer;

enum class WalkMode {
  // moves from first to last path node and stops
  ONE_SHOT,
  // moves from first to last node then in reverse order back to first
  PING_PONG,
  // moves from last node back to the first node
  CIRCULAR
};

WalkMode string_to_walk_mode(const std::string& mode_string);
std::string walk_mode_to_string(WalkMode walk_mode);

class Path final
{
public:
  /** Helper class that stores an individual node of a Path */
  class Node
  {
  public:
    Vector position; /**< the position of this node */
    float time; /**< time (in seconds) to get from this node to next node */
    float speed; /**< speed (in px/seconds); editor use only */
    EasingMode easing; /**< speed variations during travel
            (constant speed, start slow and go progressively quicker, etc.) */

    Node() :
      position(0.0f, 0.0f),
      time(),
      speed(),
      easing()
    {}
  };

public:
  Path();
  Path(const Vector& pos);

  void read(const ReaderMapping& reader);
  void save(Writer& writer);

  Vector get_base() const;

  /** returns Node index nearest to reference_point or -1 if not applicable */
  int get_nearest_node_no(const Vector& reference_point) const;

  /** returns Node index farthest from reference_point or -1 if not applicable */
  int get_farthest_node_no(const Vector& reference_point) const;

  /** Moves all nodes by given shift. */
  void move_by(const Vector& shift);

  /** Puts node markers to the nodes to edit them. */
  void edit_path();

  /** Returns false when has no nodes */
  bool is_valid() const;

  const std::vector<Node>& get_nodes() const { return m_nodes; }

public:
  std::vector<Node> m_nodes;

  WalkMode m_mode;

private:
  Path(const Path&) = delete;
  Path& operator=(const Path&) = delete;
};

#endif

/* EOF */
