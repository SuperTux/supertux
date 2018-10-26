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

#include "editor/editor.hpp"

NodeMarker::NodeMarker (Path* path_, std::vector<Path::Node>::iterator node_iterator, size_t id_) :
  path(path_),
  node(node_iterator),
  id(id_)
{
  set_pos(node->position - Vector(8, 8));
}

void NodeMarker::update_iterator() {
  if (id >= path->m_nodes.size()) {
    remove_me();
  } else {
    node = path->m_nodes.begin() + id;
  }
}

Vector NodeMarker::get_point_vector() const {
  std::vector<Path::Node>::iterator next_node = node + 1;
  if (next_node == path->m_nodes.end()) {
    if (path->m_mode == Path::CIRCULAR || path->m_mode == Path::UNORDERED) {
      //loop to the first node
      return path->m_nodes.begin()->position - node->position;
    } else {
      return Vector(0,0);
    }
  } else {
    //point to the next node
    return next_node->position - node->position;
  }
}

Vector NodeMarker::get_offset() const {
  return Vector(8, 8);
}

void NodeMarker::move_to(const Vector& pos) {
  MovingObject::move_to(pos);
  node->position = m_bbox.get_middle();
}

void NodeMarker::editor_delete() {
  path->m_nodes.erase(node);
  Editor::current()->update_node_iterators();
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
