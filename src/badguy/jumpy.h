#ifndef __JUMPY_H__
#define __JUMPY_H__

#include "badguy.h"

class Jumpy : public BadGuy
{
public:
  Jumpy(const lisp::Lisp& reader);

  virtual HitResponse collision_solid(GameObject& other,
      const CollisionHit& hit);

  virtual void write(lisp::Writer& writer);
};

#endif

