
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// by Ricardo Cruz <rick2@aeiou.pt>

#include "screen.h"
#include "mousecursor.h"

MouseCursor::MouseCursor(std::string cursor_file, int frames)
{
texture_load(&cursor,cursor_file.c_str(),USE_ALPHA);

cur_state = MC_NORMAL;
cur_frame = 0;
tot_frames = frames;

timer_init(&timer, false);
timer_start(&timer,MC_FRAME_PERIOD);

SDL_ShowCursor(SDL_DISABLE);
}

MouseCursor::~MouseCursor()
{
  texture_free(&cursor);

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

void MouseCursor::draw(int x, int y)
{
int w,h;
w = cursor.w / tot_frames;
h = cursor.h / MC_STATES_NB;

if(timer_get_left(&timer) < 0 && tot_frames > 1)
  {
  cur_frame++;
  if(cur_frame++ >= tot_frames)
    cur_frame = 0;

  timer_start(&timer,MC_FRAME_PERIOD);
  }

texture_draw_part(&cursor, w*cur_frame, h*cur_state , x, y, w, h);
}
