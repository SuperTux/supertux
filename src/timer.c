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

#include "SDL.h"
#include "defines.h"
#include "timer.h"

unsigned int st_pause_ticks, st_pause_count;

unsigned int st_get_ticks(void)
{
  if(st_pause_count != 0)
    return SDL_GetTicks() - st_pause_ticks - SDL_GetTicks() + st_pause_count;
  else
    return SDL_GetTicks() - st_pause_ticks;
}

void st_pause_ticks_init(void)
{
  st_pause_ticks = 0;
  st_pause_count = 0;
}

void st_pause_ticks_start(void)
{
  st_pause_count = SDL_GetTicks();
}

void st_pause_ticks_stop(void)
{
  st_pause_ticks += SDL_GetTicks() - st_pause_count;
  st_pause_count = 0;
}

void timer_init(timer_type* ptimer, int st_ticks)
{
  ptimer->period = 0;
  ptimer->time = 0;

  if(st_ticks == YES)
    ptimer->get_ticks = st_get_ticks;
  else
    ptimer->get_ticks = SDL_GetTicks;

}

void timer_start(timer_type* ptimer, unsigned int period)
{
  ptimer->time = ptimer->get_ticks();
  ptimer->period = period;
}

void timer_stop(timer_type* ptimer)
{
  if(ptimer->get_ticks == st_get_ticks)
    timer_init(ptimer,YES);
  else
    timer_init(ptimer,NO);
}

int timer_check(timer_type* ptimer)
{
  if((ptimer->time != 0) && (ptimer->time + ptimer->period > ptimer->get_ticks()))
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
  return (ptimer->period - (ptimer->get_ticks() - ptimer->time));
}

int timer_get_gone(timer_type* ptimer)
{
  return (ptimer->get_ticks() - ptimer->time);
}

int timer_fwrite(timer_type* ptimer, FILE* fi)
{
  unsigned int diff_ticks;
  int tick_mode;
  if(ptimer->time != 0)
    diff_ticks = ptimer->get_ticks() - ptimer->time;
  else
    diff_ticks = 0;

  fwrite(&ptimer->period,sizeof(unsigned int),1,fi);
  fwrite(&diff_ticks,sizeof(unsigned int),1,fi);
  if(ptimer->get_ticks == st_get_ticks)
      tick_mode = YES;
  else
      tick_mode = NO;
  fwrite(&tick_mode,sizeof(unsigned int),1,fi);
}

int timer_fread(timer_type* ptimer, FILE* fi)
{
  unsigned int diff_ticks;
  int tick_mode;
  fread(&ptimer->period,sizeof(unsigned int),1,fi);
  fread(&diff_ticks,sizeof(unsigned int),1,fi);
  fread(&tick_mode,sizeof(unsigned int),1,fi);
  if(tick_mode == YES)
    ptimer->get_ticks = st_get_ticks;
  else
    ptimer->get_ticks = SDL_GetTicks;
  if(diff_ticks != 0)
    ptimer->time = ptimer->get_ticks() - diff_ticks;
  else
    ptimer->time = 0;

}

