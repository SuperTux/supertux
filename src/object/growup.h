#ifndef __GROWUP_H__
#define __GROWUP_H__

#include "special/moving_object.h"
#include "special/sprite.h"
#include "math/physic.h"

using namespace SuperTux;

class GrowUp : public MovingObject
{
public:
  GrowUp(const Vector& pos);
  ~GrowUp();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  
private:
  Sprite* sprite;
  Physic physic;
};

#endif

