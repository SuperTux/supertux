#ifndef __MRBOMB_H__
#define __MRBOMB_H__

#include "badguy.h"

class MrBomb : public BadGuy
{
public:
  MrBomb(LispReader& reader);

  void activate();
  void write(LispWriter& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);

protected:
  bool collision_squished(Player& player);
};

#endif

