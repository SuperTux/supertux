//  SuperTux
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

#include "object/path_walker.hpp"

#include <math.h>

PathWalker::PathWalker(const Path* path, bool running)
  : path(path), running(running), current_node_nr(0), next_node_nr(0), stop_at_node_nr(running?-1:0), node_time(0),
    walking_speed(1.0)
{
  node_mult = 1 / path->nodes[0].time;
  next_node_nr = path->nodes.size() > 1 ? 1 : 0;
}

PathWalker::~PathWalker()
{
}

Vector
PathWalker::advance(float elapsed_time)
{
  if (!running) return path->nodes[current_node_nr].position;

  assert(elapsed_time >= 0);

  elapsed_time *= fabsf(walking_speed);

  const Path::Node* current_node = & (path->nodes[current_node_nr]);
  while(node_time + elapsed_time * node_mult >= 1) {
    elapsed_time -= (1 - node_time) / node_mult;

    if(walking_speed > 0) {
      advance_node();
    } else if(walking_speed < 0) {
      goback_node();
    }

    current_node = & (path->nodes[current_node_nr]);
    node_time = 0;
    if(walking_speed > 0) {
      node_mult = 1 / current_node->time;
    } else {
      node_mult = 1 / path->nodes[next_node_nr].time;
    }
  }

  const Path::Node* next_node = & (path->nodes[next_node_nr]);
  node_time += elapsed_time * node_mult;

  Vector new_pos = current_node->position +
    (next_node->position - current_node->position) * node_time;

  return new_pos;
}

void
PathWalker::goto_node(int node_no)
{
  if (node_no == stop_at_node_nr) return;
  running = true;
  stop_at_node_nr = node_no;
}

void
PathWalker::start_moving()
{
  running = true;
  stop_at_node_nr = -1;
}

void
PathWalker::stop_moving()
{
  stop_at_node_nr = next_node_nr;
}

void
PathWalker::advance_node()
{
  current_node_nr = next_node_nr;
  if (static_cast<int>(current_node_nr) == stop_at_node_nr) running = false;

  if(next_node_nr + 1 < path->nodes.size()) {
    next_node_nr++;
    return;
  }

  switch(path->mode) {
    case Path::ONE_SHOT:
      next_node_nr = path->nodes.size() - 1;
      walking_speed = 0;
      return;

    case Path::PING_PONG:
      walking_speed = -walking_speed;
      next_node_nr = path->nodes.size() > 1 ? path->nodes.size() - 2 : 0;
      return;

    case Path::CIRCULAR:
      next_node_nr = 0;
      return;
  }

  // we shouldn't get here
  assert(false);
  next_node_nr = path->nodes.size() - 1;
  walking_speed = 0;
}

void
PathWalker::goback_node()
{
  current_node_nr = next_node_nr;

  if(next_node_nr > 0) {
    next_node_nr--;
    return;
  }

  switch(path->mode) {
    case Path::PING_PONG:
      walking_speed = -walking_speed;
      next_node_nr = path->nodes.size() > 1 ? 1 : 0;
      return;
    default:
      break;
  }

  assert(false);
  next_node_nr = 0;
  walking_speed = 0;
}
