#ifndef __SPIKY_H__
#define __SPIKY_H__

#include "badguy.h"

class Spiky : public BadGuy
{
public:
  Spiky(LispReader& reader);

  void activate();
  void write(LispWriter& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
};

#endif
