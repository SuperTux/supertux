#ifndef __COIN_H__
#define __COIN_H__

#include "special/moving_object.h"

namespace SuperTux {
  class Sprite;
}

using namespace SuperTux;

class Coin : public MovingObject
{
public:
  Coin(const Vector& pos);
  ~Coin();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

  void collect();

private:
  Sprite* sprite;
};

#endif

