#ifndef __SPIKE_H__
#define __SPIKE_H__

#include "badguy.h"

class Spike : public BadGuy
{
public:
  enum Direction {
    NORTH=0, SOUTH, WEST, EAST
  };
  Spike(const Vector& pos, Direction dir);
  Spike(LispReader& reader);

  void active_action(float elapsed_time);
  void write(LispWriter& writer);
  void kill_fall();
private:
  void set_direction(Direction dir);
  Direction spikedir;
};

#endif

