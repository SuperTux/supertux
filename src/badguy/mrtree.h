#ifndef __MRTREE_H__
#define __MRTREE_H__

#include "badguy.h"

class MrTree : public BadGuy
{
public:
  MrTree(const lisp::Lisp& reader);

  void activate();
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);

protected:
  enum MyState {
    STATE_BIG, STATE_NORMAL
  };
  MyState mystate;
  
  bool collision_squished(Player& player);
};

#endif
