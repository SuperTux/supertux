#ifndef __SNOWBALL_H__
#define __SNOWBALL_H__

#include "badguy.h"

class SnowBall : public BadGuy
{
public:
  SnowBall(LispReader& reader);

  void activate();
  void write(LispWriter& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  
protected:
  bool collision_squished(Player& player);
};

#endif

