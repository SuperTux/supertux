//  $Id$
// 
//  SuperTux Path
//  Copyright (C) 2005 Philipp <balinor@pnxs.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "path.hpp"

#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "object_factory.hpp"
#include "msg.hpp"

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

Path::Path()
{
}

Path::~Path()
{
}

void
Path::read(const lisp::Lisp& reader)
{
  lisp::ListIterator iter(&reader);

  mode = CIRCULAR;
  while(iter.next()) {
    if(iter.item() == "mode") {
      std::string mode_string;
      if(!iter.value()->get(mode_string))
        throw new std::runtime_error("Pathmode not a string");

      if(mode_string == "oneshot")
        mode = ONE_SHOT;
      else if(mode_string == "pingpong")
        mode = PING_PONG;
      else if(mode_string == "circular")
        mode = CIRCULAR;
      else {
        std::ostringstream msg;
        msg << "Unknown pathmode '" << mode_string << "' found";
        throw new std::runtime_error(msg.str());
      }
      continue;
    }
    
    if(iter.item() != "node") {
      msg_warning("unknown token '" << iter.item() << "' in Path nodes list. Ignored.");
      continue;
    }
    const lisp::Lisp* node_lisp = iter.lisp();

    // each new node will inherit all values from the last one
    Node node;
    node.time = 1;
    if( (!node_lisp->get("x", node.position.x) ||
          !node_lisp->get("y", node.position.y)))
      throw new std::runtime_error("Path node without x and y coordinate specified");
    node_lisp->get("time", node.time);

    if(node.time <= 0)
      throw std::runtime_error("Path node with non-positive time");

    nodes.push_back(node);
  }

  if (nodes.empty())
    throw std::runtime_error("Path with zero nodes");
}

void
Path::write(lisp::Writer& writer)
{
  writer.start_list("path");

  switch(mode) {
    case ONE_SHOT:
      writer.write_string("mode", "oneshot");
      break;
    case PING_PONG:
      writer.write_string("mode", "pingpong");
      break;
    case CIRCULAR:
      writer.write_string("mode", "circular");
      break;
    default:
      msg_warning("Don't know how to write mode " << (int) mode << " ?!?");
      break;
  }

  for (size_t i=0; i < nodes.size(); i++) {
    const Node& node = nodes[i];

    writer.start_list("node");
    writer.write_float("x", node.position.x);
    writer.write_float("y", node.position.y);
    writer.write_float("time", node.time);

    writer.end_list("node");
  }

  writer.end_list("path");
}

Vector
Path::get_base() const
{
  if(nodes.empty())
    return Vector(0, 0);
  
  return nodes[0].position;
}

