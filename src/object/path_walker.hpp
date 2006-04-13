//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __PATH_WALKER_HPP__
#define __PATH_WALKER_HPP__

#include "path.hpp"
#include "math/vector.hpp"
#include "game_object.hpp"
#include "lisp/lisp.hpp"
#include "serializable.hpp"

/**
 * A walker that travels along a path
 */
class PathWalker
{
public:
  PathWalker(const Path* path);
  virtual ~PathWalker();

  /**
   * advanves the path walker on the path and returns the position delta
   * to the last position
   */
  virtual Vector advance(float elapsed_time);

private:
  void advance_node();
  void goback_node();
  
  const Path* path;

  size_t current_node_nr;
  size_t next_node_nr;

  Vector last_pos;

  /**
   * the position between the current node and the next node as fraction
   * between 0 and 1
   */
  float node_time;
  float node_mult;

  float walking_speed;
};

#endif
