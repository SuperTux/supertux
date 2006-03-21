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

#include "path.hpp"

#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "object_factory.hpp"

#include <assert.h>
#include <iostream>
#include <stdexcept>

// snap to destination if within EPSILON pixels
#define EPSILON 1.5

Path::Path(const lisp::Lisp& reader)
{
  circular = true;
  forward = true;

  if (!reader.get("name", name)) throw std::runtime_error("Path without name");
  reader.get("circular", circular);
  reader.get("forward", forward);

  const lisp::Lisp* nodes_lisp = reader.get_lisp("nodes");
  if(!nodes_lisp) throw std::runtime_error("Path without nodes");

  lisp::ListIterator iter(nodes_lisp);

  PathNode node;
  node.time = 1;

  while(iter.next()) {
    if(iter.item() != "node") {
      std::cerr << "Warning: unknown token '" << iter.item() << "' in Path nodes list. Ignored." << std::endl;
      continue;
    }
    const lisp::Lisp* node_lisp = iter.lisp();

    // each new node will inherit all values from the last one
    node_lisp->get("x", node.position.x);
    node_lisp->get("y", node.position.y);
    node_lisp->get("time", node.time);

    if(node.time <= 0) throw std::runtime_error("Path node with non-positive time");

    pathNodes.push_back(node);
  }

  if (pathNodes.size() < 1) throw std::runtime_error("Path with zero nodes");

  // initial position and velocity will be set with the first update, as timeToGo is initialized to 0.
  destinationNode = 0;

  // register this path for lookup:
  registry[name] = this;
}

Path::~Path()
{
  registry.erase(name);
}

	void
Path::update(float elapsed_time)
{

  // advance to next node at scheduled time
  if (timeToGo <= 0) {
    position = pathNodes[destinationNode].position;

    // set destinationNode to next node
    if (forward) {
      destinationNode++;
      if (destinationNode >= (int)pathNodes.size()) {
	if (circular) {
	  destinationNode = 0;
	} else {
	  destinationNode = (int)pathNodes.size()-1;
	}
      }
    } else {
      destinationNode--;
      if (destinationNode < 0) {
	if (circular) {
	  destinationNode = (int)pathNodes.size()-1;
	} else {
	  destinationNode = 0;
	}
      }
    }

    PathNode dn = pathNodes[destinationNode];
    timeToGo = dn.time;
    velocity = (dn.position - position) / timeToGo;
  }

  // move according to stored velocity
  last_movement = velocity * elapsed_time;
  position += last_movement;
  timeToGo -= elapsed_time;

  // stop when we arrive at our destination
  PathNode dn = pathNodes[destinationNode];
  if ((position - dn.position).norm() < EPSILON) {
    velocity = Vector(0,0);
  }

}

void
Path::draw(DrawingContext& )
{
  // TODO: Add a visible flag, draw the path if true
}

const Vector&
Path::GetPosition() {
  return position;
}

const Vector&
Path::GetLastMovement() {
  return last_movement;
}


//////////////////////////////////////////////////////////////////////////////
// static stuff

std::map<std::string,Path*> Path::registry;

Path*
Path::GetByName(const std::string& name) {
  return registry[name];
}

IMPLEMENT_FACTORY(Path, "path");
