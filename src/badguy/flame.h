#ifndef __FLAME_H__
#define __FLAME_H__

#include "badguy.h"

class Flame : public BadGuy
{
public:
  Flame(LispReader& reader);

  void write(LispWriter& write);
  void active_action(float elapsed_time);
  void kill_fall();

private:
  float angle;
  float radius;
  float speed;
};

#endif

