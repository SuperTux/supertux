//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include <iostream>
#include "collision_grid.hpp"
#include "collision.hpp"
#include "sector.hpp"
#include "collision_grid_iterator.hpp"

static const float DELTA = .001;

CollisionGrid::CollisionGrid(float newwidth, float newheight)
  : width(newwidth), height(newheight), cell_width(128), cell_height(128),
    iterator_timestamp(0)
{
  cells_x = size_t(width / cell_width) + 1;
  cells_y = size_t(height / cell_height) + 1;
  grid.resize(cells_x * cells_y);
}

CollisionGrid::~CollisionGrid()
{
  for(GridEntries::iterator i = grid.begin(); i != grid.end(); ++i) {
    GridEntry* entry = *i;
    while(entry) {
      GridEntry* nextentry = entry->next;
      delete entry;
      entry = nextentry;
    }
  }
}

void
CollisionGrid::add_object(MovingObject* object)
{
#ifdef DEBUG
  // make sure the object isn't already in the grid
  for(Objects::iterator i = objects.begin(); i != objects.end(); ++i) {
    ObjectWrapper* wrapper = *i;
    if(wrapper->object == object)
      assert(false);
  }
  assert(object != 0);
#endif
  
  ObjectWrapper* wrapper = new ObjectWrapper;
  wrapper->object = object;
  wrapper->timestamp = 0;
  wrapper->dest = object->bbox;
  objects.push_back(wrapper);
  wrapper->id = objects.size()-1;
  
  const Rect& bbox = object->bbox;
  for(float y = bbox.p1.y; y < bbox.p2.y; y += cell_height) {
    for(float x = bbox.p1.x; x < bbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }
      GridEntry* entry = new GridEntry;
      entry->object_wrapper = wrapper;
      entry->next = grid[gridy*cells_x + gridx];
      grid[gridy*cells_x + gridx] = entry;
    }
  }
}

void
CollisionGrid::remove_object(MovingObject* object)
{
  ObjectWrapper* wrapper = 0;
  for(Objects::iterator i = objects.begin(); i != objects.end(); ++i) {
    if((*i)->object == object) {
      wrapper = *i;
      objects.erase(i);
      break;
    }
  }
#ifdef DEBUG
  assert(wrapper != 0);
#else
  if(wrapper == 0) {
    std::cerr << "Tried to remove nonexistant object!\n";
    return;
  }
#endif
  
  const Rect& bbox = wrapper->dest;
  for(float y = bbox.p1.y; y < bbox.p2.y; y += cell_height) {
    for(float x = bbox.p1.x; x < bbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }
      remove_object_from_gridcell(gridy*cells_x + gridx, wrapper);
    }
  }

  delete wrapper;
}

void
CollisionGrid::move_object(ObjectWrapper* wrapper)
{
  // FIXME not optimal yet... should leave the gridcells untouched that don't
  // need to be changed.
  const Rect& obbox = wrapper->dest;
  for(float y = obbox.p1.y; y < obbox.p2.y; y += cell_height) {
    for(float x = obbox.p1.x; x < obbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0  ||
         gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }
      remove_object_from_gridcell(gridy*cells_x + gridx, wrapper);
    }
  }

  const Rect& nbbox = wrapper->object->bbox;
  for(float y = nbbox.p1.y; y < nbbox.p2.y; y += cell_height) {
    for(float x = nbbox.p1.x; x < nbbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }

      GridEntry* entry = new GridEntry;
      entry->object_wrapper = wrapper;
      entry->next = grid[gridy*cells_x + gridx];
      grid[gridy*cells_x + gridx] = entry;
    }
  }

  wrapper->dest = nbbox;
}

void
CollisionGrid::check_collisions()
{
  std::vector<ObjectWrapper*> moved_objects;
  
  CollisionGridIterator iter(*this, Sector::current()->get_active_region());
  while(ObjectWrapper* wrapper = iter.next_wrapper()) {
    MovingObject* object = wrapper->object;
    if(!object->is_valid())
      continue;
    if(object->get_group() == COLGROUP_DISABLED) {
      object->bbox.move(object->movement);
      object->movement = Vector(0, 0);
      moved_objects.push_back(wrapper);
      continue;
    }

    // hack for now...
    Sector::current()->collision_tilemap(object, 0);
    
    collide_object(wrapper);

    if(object->movement != Vector(0, 0)) {
      object->bbox.move(object->movement);
      object->movement = Vector(0, 0);
      moved_objects.push_back(wrapper);
    }
  }

  for(std::vector<ObjectWrapper*>::iterator i = moved_objects.begin();
      i != moved_objects.end(); ++i) {
    move_object(*i);
  }
}

void
CollisionGrid::collide_object(ObjectWrapper* wrapper)
{
  iterator_timestamp++;

  const Rect& bbox = wrapper->object->bbox;
  for(float y = bbox.p1.y - cell_height; y < bbox.p2.y + cell_height; y += cell_height) {
    for(float x = bbox.p1.x - cell_width; x < bbox.p2.x + cell_width; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        //std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }
  
      for(GridEntry* entry = grid[gridy*cells_x + gridx]; entry;
          entry = entry->next) {
        ObjectWrapper* wrapper2 = entry->object_wrapper;
        // only check each object once (even if it is in multiple cells)
        if(wrapper2->timestamp == iterator_timestamp)
          continue;
        // don't collide with objects we already collided with
        if(wrapper2->id <= wrapper->id)
          continue;

        wrapper->timestamp = iterator_timestamp;
        collide_object_object(wrapper, wrapper2);
      }
    }
  }
}

void
CollisionGrid::collide_object_object(ObjectWrapper* wrapper,
    ObjectWrapper* wrapper2)
{
  CollisionHit hit;
  MovingObject* object1 = wrapper->object;
  MovingObject* object2 = wrapper2->object;
  
  Rect dest1 = object1->get_bbox();
  dest1.move(object1->get_movement());
  Rect dest2 = object2->get_bbox();
  dest2.move(object2->get_movement());

  Vector movement = object1->get_movement() - object2->get_movement();
  if(Collision::rectangle_rectangle(hit, dest1, movement, dest2)) {
    HitResponse response1 = object1->collision(*object2, hit);
    hit.normal *= -1;
    HitResponse response2 = object2->collision(*object1, hit);

    if(response1 != CONTINUE) {
      if(response1 == ABORT_MOVE)
        object1->movement = Vector(0, 0);
      if(response2 == CONTINUE)
        object2->movement += hit.normal * (hit.depth + DELTA);
    } else if(response2 != CONTINUE) {
      if(response2 == ABORT_MOVE)
        object2->movement = Vector(0, 0);
      if(response1 == CONTINUE)
        object1->movement += -hit.normal * (hit.depth + DELTA);
    } else {
      object1->movement += -hit.normal * (hit.depth/2 + DELTA);
      object2->movement += hit.normal * (hit.depth/2 + DELTA);
    }
  }
}

void
CollisionGrid::remove_object_from_gridcell(int gridcell, ObjectWrapper* wrapper)
{
  GridEntry* lastentry = 0;
  GridEntry* entry = grid[gridcell];

  while(entry) {
    if(entry->object_wrapper == wrapper) {
      if(lastentry == 0) {
        grid[gridcell] = entry->next;
      } else {
        lastentry->next = entry->next;
      }
      delete entry;
      return;
    }

    lastentry = entry;
    entry = entry->next;
  };

  std::cerr << "Couldn't find object in cell.\n";
}

