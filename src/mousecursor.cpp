//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
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

#include "screen.h"
#include "mousecursor.h"

MouseCursor* MouseCursor::current_ = 0;

MouseCursor::MouseCursor(std::string cursor_file, int frames) : mid_x(0), mid_y(0)
{
  cursor = new Surface(cursor_file, USE_ALPHA);
  
  cur_state = MC_NORMAL;
  cur_frame = 0;
  tot_frames = frames;

  timer.init(false);
  timer.start(MC_FRAME_PERIOD);
  
  SDL_ShowCursor(SDL_DISABLE);
}

MouseCursor::~MouseCursor()
{
  delete cursor;

  SDL_ShowCursor(SDL_ENABLE);
}

int MouseCursor::state()
{
  return cur_state;
}

void MouseCursor::set_state(int nstate)
{
  cur_state = nstate;
}

void MouseCursor::set_mid(int x, int y)
{
  mid_x = x;
  mid_y = y;
}

void MouseCursor::draw()
{
  int x,y,w,h;
  Uint8 ispressed = SDL_GetMouseState(&x,&y);
  w = cursor->w / tot_frames;
  h = cursor->h / MC_STATES_NB;
  if(ispressed &SDL_BUTTON(1) || ispressed &SDL_BUTTON(2))
    {
      if(cur_state != MC_CLICK)
        {
          state_before_click = cur_state;
          cur_state = MC_CLICK;
        }
    }
  else
    {
      if(cur_state == MC_CLICK)
        cur_state = state_before_click;
    }

  if(timer.get_left() < 0 && tot_frames > 1)
    {
      cur_frame++;
      if(cur_frame++ >= tot_frames)
        cur_frame = 0;

      timer.start(MC_FRAME_PERIOD);
    }

  cursor->draw_part(w*cur_frame, h*cur_state , x-mid_x, y-mid_y, w, h);
}
