#include "moving_object.h"

MovingObject::MovingObject()
{
  base.x = base.y = base.width = base.height = 0;
  old_base = base;
}

MovingObject::~MovingObject()
{
}

