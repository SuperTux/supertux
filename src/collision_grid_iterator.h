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
#ifndef __COLLISION_GRID_ITERATOR_H__
#define __COLLISION_GRID_ITERATOR_H__

#include "math/rect.h"

class CollisionGrid;

class CollisionGridIterator
{
public:
  CollisionGridIterator(CollisionGrid& newgrid, const Rect& bbox)
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

