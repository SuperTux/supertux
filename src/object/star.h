#ifndef __STAR_H__
#define __STAR_H__

#include "special/moving_object.h"
#include "special/sprite.h"
#include "math/physic.h"

using namespace SuperTux;

class Star : public MovingObject
{
public:
  Star(const Vector& pos);
  ~Star();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  Sprite* sprite;
  Physic physic;
};

#endif
