#ifndef __COLLISION_GRID_H__
#define __COLLISION_GRID_H__

#include <vector>
#include "special/moving_object.h"

using namespace SuperTux;

/**
 * A rectangular grid to keep track of all moving game objects. It allows fast
 * queries for all objects in a rectangular area.
 */
class CollisionGrid
{
public:
  CollisionGrid(float width, float height);
  ~CollisionGrid();

  void add_object(MovingObject* object);
  void remove_object(MovingObject* object);
  void move_object(MovingObject* object);

  void check_collisions();

private:
  struct ObjectWrapper
  {
    MovingObject* object;
    Rectangle dest;
    /** (pseudo) timestamp. When reading from the grid the timestamp is
     * changed so that you can easily avoid reading an object multiple times
     * when it is in several cells that you check.
     */
    int timestamp;
    /// index in the objects vector
    int id;
  };
 
  /** Element for the single linked list in each grid cell */
  struct GridEntry
  {
    GridEntry* next;
    ObjectWrapper* object_wrapper;
  };

  void remove_object_from_gridcell(int gridcell, MovingObject* object);
  void collide_object(ObjectWrapper* wrapper);
  void collide_object_object(ObjectWrapper* wrapper, ObjectWrapper* wrapper2);
  
  typedef std::vector<GridEntry*> GridEntries;
  GridEntries grid;
  typedef std::vector<ObjectWrapper*> Objects;
  Objects objects;
  size_t cells_x, cells_y;
  float width;
  float height;
  float cell_width;
  float cell_height;
};

extern CollisionGrid* bla;

#endif

