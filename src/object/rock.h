#ifndef __ROCK_H__
#define __ROCK_H__

#include "special/moving_object.h"
#include "math/physic.h"
#include "lisp/lisp.h"
#include "portable.h"
#include "serializable.h"

namespace SuperTux {
  class Sprite;
}

class Rock : public MovingObject, public Portable, public Serializable
{
public:
  Rock(const lisp::Lisp& reader);
  virtual ~Rock();

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void action(float elapsed_time);
  void draw(DrawingContext& context);
  void write(lisp::Writer& writer);
    
  void grab(MovingObject& object, const Vector& pos);

private:
  bool grabbed;
  Sprite* sprite;
  Physic physic;
};

#endif
