#ifndef __MRROCKET_H__
#define __MRROCKET_H__

#include "badguy.h"
#include "timer.h"
#include "rocketexplosion.h"

class MrRocket : public BadGuy
{
public:
  MrRocket(const lisp::Lisp& reader);
  MrRocket(float pos_x, float pos_y, Direction d);

  void activate();
  void active_action(float elapsed_time);
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);

protected:
  bool collision_squished(Player& player);
  bool set_direction;
  Direction initial_direction;
  Timer2 collision_timer;
};

#endif

