#ifndef __BELL_H__
#define __BELL_H__

#include "lisp/lisp.h"
#include "special/moving_object.h"
#include "special/sprite.h"
#include "serializable.h"

using namespace SuperTux;

/**
 * A bell: When tux touches it, it begins ringing and you will respawn at this
 * position.
 */
class Bell : public MovingObject, public Serializable
{
public:
  Bell(const lisp::Lisp& lisp);
  ~Bell();

  void write(lisp::Writer& writer);
  void action(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  Sprite* sprite;
  bool ringing;
};

#endif

