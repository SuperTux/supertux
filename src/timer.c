//
// C Implementation: timer
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <SDL/SDL.h>
#include "defines.h"
#include "timer.h"

void timer_init(timer_type* ptimer)
{
  ptimer->period = 0;
  ptimer->time = 0;
}

void timer_start(timer_type* ptimer, unsigned int period)
{
  ptimer->time = SDL_GetTicks();
  ptimer->period = period;
}

void timer_stop(timer_type* ptimer)
{
 timer_init(ptimer);
}

int timer_check(timer_type* ptimer)
{
  if(ptimer->time != 0 && ptimer->time + ptimer->period > SDL_GetTicks())
    return YES;
  else
    {
      ptimer->time = 0;
      return NO;
    }
}

int timer_started(timer_type* ptimer)
{
  if(ptimer->time != 0)
    return YES;
  else
    return NO;
}

int timer_get_left(timer_type* ptimer)
{
  return (ptimer->period - (SDL_GetTicks() - ptimer->time));
}

int timer_get_gone(timer_type* ptimer)
{
  return (SDL_GetTicks() - ptimer->time);
}
