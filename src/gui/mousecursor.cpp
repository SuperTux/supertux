//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "gui/mousecursor.hpp"

#include <SDL.h>

#include "supertux/main.hpp"
#include "video/drawing_context.hpp"

MouseCursor* MouseCursor::current_ = 0;
extern SDL_Surface* screen;

MouseCursor::MouseCursor(std::string cursor_file) : mid_x(0), mid_y(0)
{
  cursor = new Surface(cursor_file);

  cur_state = MC_NORMAL;
}

MouseCursor::~MouseCursor()
{
  delete cursor;
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

void MouseCursor::draw(DrawingContext& context)
{
  if(cur_state == MC_HIDE)
    return;

  int x,y,w,h;
  Uint8 ispressed = SDL_GetMouseState(&x,&y);

  x = int(x * float(SCREEN_WIDTH)/screen->w);
  y = int(y * float(SCREEN_HEIGHT)/screen->h);

  w = (int) cursor->get_width();
  h = (int) (cursor->get_height() / MC_STATES_NB);
  if(ispressed &SDL_BUTTON(1) || ispressed &SDL_BUTTON(2)) {
    if(cur_state != MC_CLICK) {
      state_before_click = cur_state;
      cur_state = MC_CLICK;
    }
  } else {
    if(cur_state == MC_CLICK)
      cur_state = state_before_click;
  }

  context.draw_surface_part(cursor, Vector(0, h*cur_state),
                            Vector(w, h), Vector(x-mid_x, y-mid_y), LAYER_GUI+100);
}

/* EOF */
