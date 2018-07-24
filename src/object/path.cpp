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

#include "object/path.hpp"

#include "editor/node_marker.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "util/log.hpp"

Path::Path() :
  nodes(),
  mode(CIRCULAR)
{
}

Path::Path(const Vector& pos) :
  nodes(),
  mode()
{
  Node first_node;
  first_node.position = pos;
  first_node.time = 1;
  nodes.push_back(first_node);
}

void
Path::read(const ReaderMapping& reader)
{
  auto iter = reader.get_iter();

  mode = CIRCULAR;
  while(iter.next()) {
    if(iter.get_key() == "mode") {
      std::string mode_string;
      iter.get(mode_string);
      mode = string_to_walk_mode(mode_string);
      continue;
    } else if (iter.get_key() == "node") {
      ReaderMapping node_mapping = iter.as_mapping();

      // each new node will inherit all values from the last one
      Node node;
      node.time = 1;
      if( (!node_mapping.get("x", node.position.x) ||
           !node_mapping.get("y", node.position.y)))
        throw std::runtime_error("Path node without x and y coordinate specified");
      node_mapping.get("time", node.time);

      if(node.time <= 0)
        throw std::runtime_error("Path node with non-positive time");

      nodes.push_back(node);
    } else {
      log_warning << "unknown token '" << iter.get_key() << "' in Path nodes list. Ignored." << std::endl;
    }
  }

  if (nodes.empty())
    throw std::runtime_error("Path with zero nodes");
}

void
Path::save(Writer& writer) {
  if (!is_valid()) return;

  writer.start_list("path");
  writer.write("mode", walk_mode_to_string(mode), false);

  for(auto& nod : nodes) {
    writer.start_list("node");
    writer.write("x", nod.position.x);
    writer.write("y", nod.position.y);
    writer.write("time", nod.time);
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
Path::get_nearest_node_no(const Vector& reference_point) const
{
  int nearest_node_id = -1;
  float nearest_node_dist = 0;
  int id = 0;
  for (std::vector<Node>::const_iterator i = nodes.begin(); i != nodes.end(); ++i, ++id) {
    float dist = (i->position - reference_point).norm();
    if ((nearest_node_id == -1) || (dist < nearest_node_dist)) {
      nearest_node_id = id;
      nearest_node_dist = dist;
    }
  }
  return nearest_node_id;
}

int
Path::get_farthest_node_no(const Vector& reference_point) const
{
  int farthest_node_id = -1;
  float farthest_node_dist = 0;
  int id = 0;
  for (std::vector<Node>::const_iterator i = nodes.begin(); i != nodes.end(); ++i, ++id) {
    float dist = (i->position - reference_point).norm();
    if ((farthest_node_id == -1) || (dist > farthest_node_dist)) {
      farthest_node_id = id;
      farthest_node_dist = dist;
    }
  }
  return farthest_node_id;
}

void
Path::move_by(const Vector& shift) {
  for(auto& nod : nodes) {
    nod.position += shift;
  }
}

void
Path::edit_path() {
  int id = 0;
  for(auto i = nodes.begin(); i != nodes.end(); ++i) {
    GameObjectPtr marker;
    marker = std::make_shared<NodeMarker>(this, i, id);
    Sector::current()->add_object(marker);
    id++;
  }
}

bool
Path::is_valid() const {
  return nodes.size();
}

Path::WalkMode
Path::string_to_walk_mode(const std::string& mode_string) const {
  if(mode_string == "oneshot")
    return ONE_SHOT;
  else if(mode_string == "pingpong")
    return PING_PONG;
  else if(mode_string == "circular")
    return CIRCULAR;
  else if(mode_string == "unordered")
    return UNORDERED;
  else {
    log_warning << "Unknown path mode '" << mode_string << "'found. Using oneshot instead.";
    return ONE_SHOT;
  }
}

const std::string
Path::walk_mode_to_string(const WalkMode& walk_mode) const
{
  if(walk_mode == ONE_SHOT)
    return "oneshot";
  else if(walk_mode == PING_PONG)
    return "pingpong";
  else if(walk_mode == CIRCULAR)
    return "circular";
  else if(walk_mode == UNORDERED)
    return "unordered";
  else {
    log_warning << "Unknown path mode found. Using oneshot instead.";
    return "oneshot";
  }
}

ObjectOption
Path::get_mode_option(WalkMode* mode_) {
  ObjectOption result(MN_STRINGSELECT, _("Path Mode"), mode_);
  result.select.push_back(_("one shot"));
  result.select.push_back(_("ping pong"));
  result.select.push_back(_("circular"));
  result.select.push_back(_("unordered"));
  return result;
}
/* EOF */
