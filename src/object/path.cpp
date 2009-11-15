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

//#include <config.h>

#include "object/path.hpp"

//#include <assert.h>
//#include <iostream>
//#include <sstream>
#include <stdexcept>

//#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/writer.hpp"
#include "util/log.hpp"

Path::Path() :
  nodes(),
  mode()
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
        throw std::runtime_error("Pathmode not a string");

      if(mode_string == "oneshot")
        mode = ONE_SHOT;
      else if(mode_string == "pingpong")
        mode = PING_PONG;
      else if(mode_string == "circular")
        mode = CIRCULAR;
      else {
        std::ostringstream msg;
        msg << "Unknown pathmode '" << mode_string << "' found";
        throw std::runtime_error(msg.str());
      }
      continue;
    }

    if(iter.item() != "node") {
      log_warning << "unknown token '" << iter.item() << "' in Path nodes list. Ignored." << std::endl;
      continue;
    }
    const lisp::Lisp* node_lisp = iter.lisp();

    // each new node will inherit all values from the last one
    Node node;
    node.time = 1;
    if( (!node_lisp->get("x", node.position.x) ||
          !node_lisp->get("y", node.position.y)))
      throw std::runtime_error("Path node without x and y coordinate specified");
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
      writer.write("mode", "oneshot");
      break;
    case PING_PONG:
      writer.write("mode", "pingpong");
      break;
    case CIRCULAR:
      writer.write("mode", "circular");
      break;
    default:
      log_warning << "Don't know how to write mode " << (int) mode << " ?!?" << std::endl;
      break;
  }

  for (size_t i=0; i < nodes.size(); i++) {
    const Node& node = nodes[i];

    writer.start_list("node");
    writer.write("x", node.position.x);
    writer.write("y", node.position.y);
    writer.write("time", node.time);

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

int
Path::get_nearest_node_no(Vector reference_point) const
{
  int nearest_node_id = -1;
  float nearest_node_dist = 0;
  int id = 0;
  for (std::vector<Node>::const_iterator i = nodes.begin(); i != nodes.end(); i++, id++) {
    float dist = (i->position - reference_point).norm();
    if ((nearest_node_id == -1) || (dist < nearest_node_dist)) {
      nearest_node_id = id;
      nearest_node_dist = dist;
    }
  }
  return nearest_node_id;
}

int
Path::get_farthest_node_no(Vector reference_point) const
{
  int farthest_node_id = -1;
  float farthest_node_dist = 0;
  int id = 0;
  for (std::vector<Node>::const_iterator i = nodes.begin(); i != nodes.end(); i++, id++) {
    float dist = (i->position - reference_point).norm();
    if ((farthest_node_id == -1) || (dist > farthest_node_dist)) {
      farthest_node_id = id;
      farthest_node_dist = dist;
    }
  }
  return farthest_node_id;
}

