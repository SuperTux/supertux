#ifndef __DISPENSER_H__
#define __DISPENSER_H__

#include "badguy.h"
#include "timer.h"

class Dispenser : public BadGuy
{
public:
  Dispenser(const lisp::Lisp& reader);

  void activate();
  void write(lisp::Writer& writer);
  void active_action(float elapsed_time);

protected:
  bool collision_squished(Player& player);
  void launch_badguy();
  float cycle;
  std::string badguy;
  Timer2 dispense_timer;
};

#endif

