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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "SDL.h"

#include "../special/frame_rate.h"
#include "../special/timer.h"

using namespace SuperTux;

FrameRate::FrameRate(double fps)
{
  set_fps(fps);
}

void FrameRate::start()
{
  update_time = last_update_time = Ticks::get();  
}

void FrameRate::set_fps(double fps)
{
  frame_ms = static_cast<unsigned int>(1000.f/fps);
}

double FrameRate::get()
{
  return ((double)(update_time-last_update_time))/(double)frame_ms;
}

void FrameRate::update()
{
  /* Set the time of the last update and the time of the current update */
  last_update_time = update_time;
  update_time      = Ticks::get();

  /* Pause till next frame, if the machine running the game is too fast: */
  /* FIXME: Works great for in OpenGl mode, where the CPU doesn't have to do that much. But
     the results in SDL mode aren't perfect (thought the 100 FPS are reached), even on an AMD2500+. */
  if(last_update_time >= update_time - (frame_ms+2))
    {
      SDL_Delay(frame_ms);
      update_time = Ticks::get();
    }
}

