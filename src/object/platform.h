#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "special/moving_object.h"
#include "special/sprite.h"

using namespace SuperTux;

/**
 * This class is the base class for platforms that tux can stand on
 */
class Platform : public SuperTux::MovingObject
{
public:
  Platform(const lisp::Lisp& reader);
  ~Platform();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  int state;
  Sprite* sprite;
};

#endif

