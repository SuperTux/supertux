#ifndef __NOLOK01_H__
#define __NOLOK01_H__

#include "badguy.h"
#include "timer.h"

class Nolok_01 : public BadGuy
{
public:
  Nolok_01(const lisp::Lisp& reader);
  Nolok_01(float pos_x, float pos_y);

  void activate();
  void write(lisp::Writer& writer);
  void active_action(float elapsed_time);
  void kill_fall();
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  
protected:
  bool collision_squished(Player& player);
  Timer2 action_timer;
  enum Actions { WALKING, JUMPING, SHOOTING };
  Actions action;
};

#endif

