#ifndef __FLAME_H__
#define __FLAME_H__

#include "badguy.h"

class Flame : public BadGuy
{
public:
  Flame(const lisp::Lisp& reader);

  void write(lisp::Writer& write);
  void active_action(float elapsed_time);
  void kill_fall();

private:
  float angle;
  float radius;
  float speed;
};

#endif

