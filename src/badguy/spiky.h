#ifndef __SPIKY_H__
#define __SPIKY_H__

#include "badguy.h"

class Spiky : public BadGuy
{
public:
  Spiky(const lisp::Lisp& reader);

  void activate();
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
};

#endif
