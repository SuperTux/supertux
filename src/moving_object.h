#ifndef __MOVING_OBJECT_H__
#define __MOVING_OBJECT_H__

#include "type.h"
#include "game_object.h"
#include "vector.h"
//#include "rectangle.h"

/**
 * Base class for all dynamic/moving game objects. This class contains things
 * for handling the bounding boxes and collision feedback.
 */
class MovingObject : public GameObject
{
public:
  MovingObject();
  virtual ~MovingObject();

  /** this function is called when the object collided with any other object
   */
  virtual void collision(const MovingObject& other_object, 
          int collision_type) = 0;

  base_type base;
  base_type old_base;

protected:
#if 0 // this will be used in my collision detection rewrite later
  /// the current position of the object
  Vector pos;
  /// the position we want to move until next frame
  Vector new_pos;
  /// the bounding box relative to the current position
  Rectangle bounding_box;
#endif
};

#endif

