#ifndef __POISONIVY_H__
#define __POISONIVY_H__

#include "badguy.h"

class PoisonIvy : public BadGuy
{
public:
  PoisonIvy(const lisp::Lisp& reader);
  PoisonIvy(float pos_x, float pos_y, Direction d);

  void activate();
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit);
  
protected:
  bool collision_squished(Player& player);
  bool set_direction;
  Direction initial_direction;  
};

#endif
