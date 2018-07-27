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

#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

MouseCursor* MouseCursor::current_ = 0;

MouseCursor::MouseCursor(const std::string& cursor_file,
                         const std::string& cursor_click_file,
                         const std::string& cursor_link_file) :
  m_mid_x(0),
  m_mid_y(0),
  m_state(MC_NORMAL),
  m_cursor(),
  m_icon()
{
  m_cursor.push_back(Surface::create(cursor_file));
  m_cursor.push_back(Surface::create(cursor_click_file));
  m_cursor.push_back(Surface::create(cursor_link_file));
}

void MouseCursor::set_state(MouseCursorState nstate)
{
  m_state = nstate;
}

void MouseCursor::set_mid(int x, int y)
{
  m_mid_x = x;
  m_mid_y = y;
}

void MouseCursor::set_icon(SurfacePtr icon_)
{
  m_icon = icon_;
}

void MouseCursor::draw(DrawingContext& context)
{
  if (m_state != MC_HIDE)
  {
    int x;
    int y;
    Uint8 ispressed = SDL_GetMouseState(&x, &y);

    Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(x, y);

    x = int(mouse_pos.x);
    y = int(mouse_pos.y);

    int tmp_state = m_state;
    if (ispressed & SDL_BUTTON(1) || ispressed & SDL_BUTTON(2))
    {
      tmp_state = MC_CLICK;
    }

    context.color().draw_surface(m_cursor[static_cast<int>(tmp_state)],
                                   Vector(x - m_mid_x, y - m_mid_y),
                                   LAYER_GUI + 100);

    if (m_icon) {
      context.color().draw_surface(m_icon, Vector(x - m_mid_x,
                                                    y - m_mid_y - m_icon->get_height()),
                                     LAYER_GUI + 100);
    }
  }
}

/* EOF */
