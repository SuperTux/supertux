//  $Id: path.hpp 3114 2006-03-23 23:47:04Z sommer $
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

#include <math.h>
#include "path_walker.hpp"

PathWalker::PathWalker(const Path* path)
  : path(path), current_node_nr(0), next_node_nr(0), node_time(0),
    walking_speed(1.0)
{
  last_pos = path->nodes[0].position;
  node_mult = 1 / path->nodes[0].time;
  next_node_nr = path->nodes.size() > 1 ? 1 : 0;
}

PathWalker::~PathWalker()
{
}

Vector
PathWalker::advance(float elapsed_time)
{
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
    
  Vector result = new_pos - last_pos;
  last_pos = new_pos;
  
  return result;
}

void
PathWalker::advance_node()
{
  current_node_nr = next_node_nr;

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
