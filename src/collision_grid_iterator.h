#ifndef __COLLISION_GRID_ITERATOR_H__
#define __COLLISION_GRID_ITERATOR_H__

#include "math/rectangle.h"

using namespace SuperTux;

class CollisionGrid;

class CollisionGridIterator
{
public:
  CollisionGridIterator(CollisionGrid& newgrid, const Rectangle& bbox)
    : grid(newgrid)
  {
    start_x = int(bbox.p1.x / grid.cell_width) - 2;
    if(start_x < 0)
      start_x = 0;
    x = start_x;
        
    y = int(bbox.p1.y / grid.cell_height) - 2;
    if(y < 0)
      y = 0;
    
    end_x = int(bbox.p2.x / grid.cell_width) + 2;
    if(end_x > (int) grid.cells_x)
      end_x = grid.cells_x;
    
    end_y = int(bbox.p2.y / grid.cell_height) + 2;
    if(end_y > (int) grid.cells_y)
      end_y = grid.cells_y;

    if(start_x >= end_x) {
      printf("bad region.\n");
      y = 0;
      end_y = 0;
      return;
    }

    timestamp = grid.iterator_timestamp++;
    entry = 0;
  }

  MovingObject* next()
  {
    CollisionGrid::ObjectWrapper* wrapper = next_wrapper();
    if(wrapper == 0)
      return 0;
        
    return wrapper->object;
  }

private:
  friend class CollisionGrid;

  CollisionGrid::ObjectWrapper* next_wrapper() 
  {
    CollisionGrid::ObjectWrapper* wrapper;
    
    do {
      while(entry == 0) {
        if(y >= end_y)
          return 0;
        
        entry = grid.grid[y*grid.cells_x + x];
        x++;
        if(x >= end_x) {
          x = start_x;
          y++;
        }
      }
      
      wrapper = entry->object_wrapper;
      entry = entry->next;
    } while(wrapper->timestamp == timestamp);
    
    wrapper->timestamp = timestamp;
    
    return wrapper;
  }
    
  CollisionGrid& grid;
  CollisionGrid::GridEntry* entry;
  int x, y;
  int start_x, end_x, end_y;
  int timestamp;
};

#endif

