#ifndef __SPECIALRISE_H__
#define __SPECIALRISE_H__

#include "special/moving_object.h"

using namespace SuperTux;

/**
 * special object that contains another object and slowly rises it out of a
 * bonus block.
 */
class SpecialRiser : public GameObject
{
public:
  SpecialRiser(MovingObject* child);
  ~SpecialRiser();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  float offset;
  MovingObject* child;
};

#endif

