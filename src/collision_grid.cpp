#include <config.h>

#include <iostream>
#include "collision_grid.h"
#include "special/collision.h"
#include "sector.h"
#include "collision_grid_iterator.h"

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
  
  const Rectangle& bbox = object->bbox;
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
  assert(wrapper != 0);
  
  const Rectangle& bbox = wrapper->dest;
  for(float y = bbox.p1.y; y < bbox.p2.y; y += cell_height) {
    for(float x = bbox.p1.x; x < bbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }
      remove_object_from_gridcell(gridy*cells_x + gridx, object);
    }
  }

  delete wrapper;
}

void
CollisionGrid::move_object(MovingObject* object)
{
  const Rectangle& bbox = object->bbox;
  for(float y = bbox.p1.y; y < bbox.p2.y; y += cell_height) {
    for(float x = bbox.p1.x; x < bbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
        continue;
      }
      // TODO
    }
  }
}

void
CollisionGrid::check_collisions()
{
  CollisionGridIterator iter(*this, Sector::current()->get_active_region());
  while(ObjectWrapper* wrapper = iter.next_wrapper()) {
    MovingObject* object = wrapper->object;
    if(!object->is_valid())
      continue;
    if(object->get_flags() & GameObject::FLAG_NO_COLLDET) {
      object->bbox.move(object->movement);
      object->movement = Vector(0, 0);
      continue;
    }

    // hack for now...
    Sector::current()->collision_tilemap(object, 0);
    
    collide_object(wrapper);

    object->bbox.move(object->get_movement());
    object->movement = Vector(0, 0);
  }
}

void
CollisionGrid::collide_object(ObjectWrapper* wrapper)
{
  iterator_timestamp++;

  const Rectangle& bbox = wrapper->object->bbox;
  for(float y = bbox.p1.y; y < bbox.p2.y; y += cell_height) {
    for(float x = bbox.p1.x; x < bbox.p2.x; x += cell_width) {
      int gridx = int(x / cell_width);
      int gridy = int(y / cell_height);
      if(gridx < 0 || gridy < 0 
          || gridx >= int(cells_x) || gridy >= int(cells_y)) {
        std::cerr << "Object out of range: " << gridx << ", " << gridy << "\n";
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
  
  Rectangle dest1 = object1->get_bbox();
  dest1.move(object1->get_movement());
  Rectangle dest2 = object2->get_bbox();
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
CollisionGrid::remove_object_from_gridcell(int gridcell, MovingObject* object)
{
  GridEntry* lastentry = 0;
  GridEntry* entry = grid[gridcell];

  while(entry) {
    if(entry->object_wrapper->object == object) {
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

