//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include "SDL.h"
#include "../special/timer.h"

using namespace SuperTux;

unsigned int Ticks::pause_ticks, Ticks::pause_count;

unsigned int Ticks::get(void)
{
  if(pause_count != 0)
    return /*SDL_GetTicks()*/ - pause_ticks /*- SDL_GetTicks()*/ + pause_count;
  else
    return SDL_GetTicks() - pause_ticks;
}

void Ticks::pause_init(void)
{
  pause_ticks = 0;
  pause_count = 0;
}

void Ticks::pause_start(void)
{
  if(pause_count == 0)
    pause_count = SDL_GetTicks();
}

void Ticks::pause_stop(void)
{
if(pause_count == 0)
return;

  pause_ticks += SDL_GetTicks() - pause_count;
  pause_count = 0;
}

bool Ticks::pause_started(void)
{
if(pause_count == 0)
return false;
else
return true;
}

Timer::Timer()
{
  init(true);
}

void
Timer::init(bool game_ticks)
{
  period    = 0;
  time      = 0;
  get_ticks = game_ticks ? Ticks::get : SDL_GetTicks;
}

void
Timer::start(unsigned int period_)
{
  time   = get_ticks();
  period = period_;
}

void
Timer::stop()
{
  if(get_ticks == get_ticks)
    init(true);
  else
    init(false);
}

int
Timer::check()
{
  if((time != 0) && (time + period > get_ticks()))
    return true;
  else
    {
      time = 0;
      return false;
    }
}

int
Timer::started()
{
  if(time != 0)
    return true;
  else
    return false;
}

int
Timer::get_left()
{
  return (period - (get_ticks() - time));
}

int
Timer::get_gone()
{
  return (get_ticks() - time);
}

void
Timer::fwrite(FILE* fi)
{
  unsigned int diff_ticks;
  int tick_mode;
  if(time != 0)
    diff_ticks = get_ticks() - time;
  else
    diff_ticks = 0;

  ::fwrite(&period,sizeof(unsigned int),1,fi);
  ::fwrite(&diff_ticks,sizeof(unsigned int),1,fi);
  if(get_ticks == get_ticks)
      tick_mode = true;
  else
      tick_mode = false;
  ::fwrite(&tick_mode,sizeof(unsigned int),1,fi);
}

void
Timer::fread(FILE* fi)
{
  unsigned int diff_ticks;
  int tick_mode;

  ::fread(&period,sizeof(unsigned int),1,fi);
  ::fread(&diff_ticks,sizeof(unsigned int),1,fi);
  ::fread(&tick_mode,sizeof(unsigned int),1,fi);

  if (tick_mode)
    get_ticks = get_ticks;
  else
    get_ticks = SDL_GetTicks;

  if (diff_ticks != 0)
    time = get_ticks() - diff_ticks;
  else
    time = 0;

}

/* EOF */
