#ifndef __UNSTABLE_TILE_H__
#define __UNSTABLE_TILE_H__

#include "special/moving_object.h"
#include "lisp/lisp.h"
#include "math/physic.h"
#include "timer.h"

namespace SuperTux {
  class Sprite;
}
class Player;

using namespace SuperTux;

/** A tile that starts falling down if tux stands to long on it */
class UnstableTile : public MovingObject
{
public:
  UnstableTile(const lisp::Lisp& lisp);
  ~UnstableTile();

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void action(float elapsed_time);
  void draw(DrawingContext& context);

private:
  Physic physic;
  Sprite* sprite;
  Timer2 timer;
  bool hit;
  bool falling;
};

#endif

