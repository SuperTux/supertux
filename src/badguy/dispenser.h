#ifndef __DISPENSER_H__
#define __DISPENSER_H__

#include "badguy.h"
#include "timer.h"

class Dispenser : public BadGuy
{
public:
  Dispenser(LispReader& reader);

  void activate();
  void write(LispWriter& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  void active_action(float elapsed_time);

protected:
  bool collision_squished(Player& player);
  float cycle;
  std::string badguy;
  Timer2 dispense_timer;
};

#endif

