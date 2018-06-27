//  SuperTux -  A Jump'n Run
//  Copyright (C) 2018 Christian Hagemeier <christian@hagemeier.ch>
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

#ifndef HEADER_SUPERTUX_MATH_QUADTREE_HPP
#define HEADER_SUPERTUX_MATH_QUADTREE_HPP
#include "math/rectf.hpp"
#include "math/size.hpp"
#include "supertux/game_object_ptr.hpp"
#include "supertux/moving_object.hpp"
#include <vector>

class QuadTree {
public:
  /**
   *  Constructs a quadtree, with the given Rectangle as bound.
   */
  QuadTree(const Rectf& r, int level = 1);
  /**
   *  Inserts obj into the Quadtree.
   */
  void insert(MovingObject *obj);
  /**
   *  Clears the tree (i.e. removes all objects).
   */
  void clear();
  /**
   *  Queries for all objects, which might intersect with obj.
   *  Fills those objects into fill.
   */
  void retrieve(MovingObject *obj, std::vector<MovingObject *> &fill);
  ~QuadTree();

private:
  void split();
  /**
   *  Determines wether obj can fit into subnode.
   *  Returns -1 if obj can only fit into the complete node.
   *  Otherwise returns index of quadrant.
   */
  int getIndex(MovingObject *obj);
  /** Maximum number of objects per node (else node is split) */
  const size_t MAX_OBJECTS = 10;
  /** Maximum number of layers (Note: Worst case tree has 4^MAX_LAYERS Nodes)
     MAX_LAYERS = 6 implies maximum number of nodes is 4096.
   */
  const int MAX_LAYERS = 6;
  /** Stores current level */
  int m_level;
  /** Saves objects for current position. */
  std::vector<MovingObject *> m_objects;
  /** Stores children at this node */
  std::vector<MovingObject *> m_onlyhere;
  /** Rectangle represented by this node. */
  Rectf m_bounds;
  /** Children. Numbered like quadrants. */
  QuadTree *next[4];

  QuadTree(
      const QuadTree &); // private copy-con, no definition (declaration-only).
  QuadTree &
  operator=(const QuadTree &); // private copy-assign, no definition either.
};
#endif
