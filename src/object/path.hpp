//  $Id$
// 
//  SuperTux Path
//  Copyright (C) 2005 Philipp <balinor@pnxs.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __PATH_HPP__
#define __PATH_HPP__

#include <string>
#include <list>
#include <map>

#include "math/vector.hpp"
#include "game_object.hpp"
#include "lisp/lisp.hpp"


/**
 * Helper class that stores an individual node of a Path
 */
class PathNode
{
public:
  Vector position; /**< position (in pixels) of this node */
  float time; /**< time (in seconds) to get to this node */
};


/**
 * Path an object can travel along. Made up of multiple nodes of type PathNode.
 */
class Path : public GameObject
{
public:
  Path(const lisp::Lisp& reader);
  ~Path();

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

  const Vector& GetPosition();
  const Vector& GetLastMovement();

  // WARNING: returns NULL if not found !
  static Path* GetByName(const std::string& name);

private:
  std::string name; /**< name this path can be referenced with, stored in PathRegistry */
  bool circular; /**< true: start with the first node once the last one has been reached. false: path will stop at last node */
  bool forward; /**< true: travel to nodes in the order they were defined. false: inverse order */
  std::vector<PathNode> pathNodes; /**< list of nodes that make up this path */

  Vector position; /**< current position */
  Vector velocity; /**< current velocity */
  Vector last_movement; /**< amount of pixels we moved in the last call to update */

  int destinationNode; /**< current destination Node */
  float timeToGo; /**< seconds until we arrive at the destination */

  static std::map<std::string,Path*> registry;
};

#endif
