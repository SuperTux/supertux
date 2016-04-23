//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "editor/node_marker.hpp"

NodeMarker::NodeMarker (Path* path_, std::vector<Path::Node>::iterator node_iterator) :
  path(path_),
  node(node_iterator)
{
  set_pos(node->position - Vector(8, 8));
}

NodeMarker::~NodeMarker() {

}

Vector NodeMarker::get_point_vector() const {
  std::vector<Path::Node>::iterator next_node = node + 1;
  if (next_node == path->nodes.end()) {
    if (path->mode == Path::CIRCULAR || path->mode == Path::UNORDERED) {
      //loop to the first node
      return path->nodes.begin()->position - node->position;
    } else {
      return Vector(0,0);
    }
  } else {
    //point to the next node
    return next_node->position - node->position;
  }
}

void NodeMarker::move_to(const Vector& pos) {
  MovingObject::move_to(pos);
  node->position = bbox.get_middle();
}

void NodeMarker::editor_delete() {
  GameObject::editor_delete();
  path->nodes.erase(node);
}

ObjectSettings NodeMarker::get_settings() {
  ObjectSettings result(_("Path Node"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Time"), &(node->time)));
  return result;
}

void NodeMarker::update(float elapsed_time) {
  set_pos(node->position - Vector(8, 8));
}

/* EOF */
