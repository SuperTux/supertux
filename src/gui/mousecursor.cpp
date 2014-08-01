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

#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/sdl/sdl_renderer.hpp"

MouseCursor* MouseCursor::current_ = 0;

MouseCursor::MouseCursor(std::string cursor_file) : 
  mouse_pos(),
  mid_x(0), 
  mid_y(0),
  state_before_click(),
  cur_state(),
  cursor()
{
  cursor = Surface::create(cursor_file);

  cur_state = MC_NORMAL;
}

MouseCursor::~MouseCursor()
{
}

int MouseCursor::state()
{
  return cur_state;
}

void MouseCursor::set_state(int nstate)
{
  cur_state = nstate;
}

void
MouseCursor::set_pos(const Vector& pos)
{
  mouse_pos = pos;
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

  // Not using coordinates from mouse, as they are in the wrong
  // coordinate system, see:
  // https://bugzilla.libsdl.org/show_bug.cgi?id=2442
  Uint8 ispressed = SDL_GetMouseState(NULL, NULL);

  int x = int(mouse_pos.x);
  int y = int(mouse_pos.y);

  int w = (int) cursor->get_width();
  int h = (int) (cursor->get_height() / MC_STATES_NB);
  if(ispressed & SDL_BUTTON(1) || ispressed & SDL_BUTTON(2)) {
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
