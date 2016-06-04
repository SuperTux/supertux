//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include <math.h>

#include "editor/scroller.hpp"

#include "editor/editor.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

namespace {

const float TOPLEFT = 16;
const float MIDDLE = 48;
const float BOTTOMRIGHT = 80;
const float SIZE = 96;

}

EditorScroller::EditorScroller() :
  hidden(false),
  scrolling(),
  scrolling_vec(0, 0),
  mouse_pos(0, 0)
{
}

EditorScroller::~EditorScroller()
{
}

bool
EditorScroller::can_scroll() {
  return scrolling && mouse_pos.x < SIZE && mouse_pos.y < SIZE;
}

void
EditorScroller::draw(DrawingContext& context) {
  if (hidden) return;

  context.draw_filled_rect(Rectf(Vector(0, 0), Vector(SIZE, SIZE)),
                           Color(0.9f, 0.9f, 1.0f, 0.6f),
                           MIDDLE, LAYER_GUI-10);
  context.draw_filled_rect(Rectf(Vector(40, 40), Vector(56, 56)),
                           Color(0.9f, 0.9f, 1.0f, 0.6f),
                           8, LAYER_GUI-20);
  if (can_scroll()) {
    draw_arrow(context, mouse_pos);
  }

  draw_arrow(context, Vector(TOPLEFT, MIDDLE));
  draw_arrow(context, Vector(BOTTOMRIGHT, MIDDLE));
  draw_arrow(context, Vector(MIDDLE, TOPLEFT));
  draw_arrow(context, Vector(MIDDLE, BOTTOMRIGHT));
}

void
EditorScroller::draw_arrow(DrawingContext& context, Vector pos) {
  Vector dir = pos - Vector(MIDDLE, MIDDLE);
  if (dir.x != 0 || dir.y != 0) {
    // draw a triangle
    dir = dir.unit() * 8;
    Vector dir2 = Vector(-dir.y, dir.x);
    context.draw_triangle(pos + dir, pos - dir + dir2, pos - dir - dir2,
                          Color(1, 1, 1, 0.5), LAYER_GUI-20);
  }
}

void
EditorScroller::update(float elapsed_time) {
  if (hidden) return;
  if (!can_scroll()) return;

  float horiz_scroll = scrolling_vec.x * elapsed_time;
  float vert_scroll = scrolling_vec.y * elapsed_time;
  auto editor = Editor::current();

  if (horiz_scroll < 0)
    editor->scroll_left(-horiz_scroll);
  else if (horiz_scroll > 0)
    editor->scroll_right(horiz_scroll);
  else {}

  if (vert_scroll < 0)
    editor->scroll_up(-vert_scroll);
  else if (vert_scroll > 0)
    editor->scroll_down(vert_scroll);
  else {}
}

bool
EditorScroller::event(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    {
      if(ev.button.button == SDL_BUTTON_LEFT) {
        if (hidden) return false;

        if (mouse_pos.x < SIZE && mouse_pos.y < SIZE) {
          scrolling = true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } break;

    case SDL_MOUSEBUTTONUP:
      scrolling = false;
      return false;
      break;

    case SDL_MOUSEMOTION:
    {
      if (hidden) return false;

      mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      if (mouse_pos.x < SIZE && mouse_pos.y < SIZE) {
        scrolling_vec = mouse_pos - Vector(MIDDLE, MIDDLE);
        if (scrolling_vec.x != 0 || scrolling_vec.y != 0) {
          float norm = scrolling_vec.norm();
          scrolling_vec *= pow(M_E, norm/16 - 1);
        }
      }
      return false;
    } break;

    case SDL_KEYDOWN:
      if (ev.key.keysym.sym == SDLK_F9) {
        hidden = !hidden;
      }
      return false;
      break;

    default:
      return false;
      break;
  }
  return true;
}

/* EOF */
