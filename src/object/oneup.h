#ifndef __ONEUP_H__
#define __ONEUP_H__

#include "special/moving_object.h"
#include "special/sprite.h"
#include "math/physic.h"

using namespace SuperTux;

class OneUp : public MovingObject
{
public:
  OneUp(const Vector& pos);
  ~OneUp();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  Sprite* sprite;
  Physic physic;
};

#endif
