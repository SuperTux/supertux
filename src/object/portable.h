#ifndef __PORTABLE_H__
#define __PORTABLE_H__

#include "special/moving_object.h"

using namespace SuperTux;

/**
 * An object that inherits from this object is considered "portable" and can
 * be carried around by the player.
 * The object has to additionally set the PORTABLE flag (this allows to
 * make the object only temporarily portable by resetting the flag)
 */
class Portable
{
public:
    /**
     * called each frame when the object has been grabbed.
     */
  virtual void grab(MovingObject& object, const Vector& pos) = 0;
};

#endif
