#ifndef __BULLET_H__
#define __BULLET_H__

#include "special/moving_object.h"
#include "math/physic.h"
#include "special/sprite.h"

using namespace SuperTux;

enum BulletsKind {
  FIRE_BULLET,
  ICE_BULLET
};

class Bullet : public MovingObject
{
public:
  Bullet(const Vector& pos, float xm, int dir, int kind);
  ~Bullet();
  
  void action(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  int kind;        
  
private:
  int life_count;
  Physic physic;
  Sprite* sprite;
};

#endif
