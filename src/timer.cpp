#include <config.h>

#include <math.h>
#include "timer.h"

float global_time = 0;

Timer2::Timer2()
  : period(0), cycle_start(0), cyclic(false)
{
}

Timer2::~Timer2()
{
}

void
Timer2::start(float period, bool cyclic)
{
  this->period = period;
  this->cyclic = cyclic;
  cycle_start = global_time;
}

bool
Timer2::check()
{
  if(period == 0)
    return false;
  
  if(global_time - cycle_start >= period) {
    if(cyclic) {
      cycle_start = global_time - fmodf(global_time - cycle_start, period);
    } else {
      period = 0;
    }
    return true;
  }

  return false;
}

