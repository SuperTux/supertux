#ifndef __JUMPY_H__
#define __JUMPY_H__

#include "badguy.h"

class Jumpy : public BadGuy
{
public:
  Jumpy(const lisp::Lisp& reader);

  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit);

  void write(lisp::Writer& writer);

private:
  HitResponse hit(const CollisionHit& hit);
};

#endif

