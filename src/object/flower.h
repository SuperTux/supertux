#ifndef __FLOWER_H__
#define __FLOWER_H__

#include "special/moving_object.h"
#include "special/sprite.h"
#include "math/physic.h"

using namespace SuperTux;

class Flower : public MovingObject
{
public:
  enum Type {
    FIREFLOWER, ICEFLOWER
  };
  Flower(const Vector& pos, Type type);
  ~Flower();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  Type type;
  Sprite* sprite;
};

#endif

