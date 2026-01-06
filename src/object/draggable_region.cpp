//  SuperTux
//  Copyright (C) 2026 Hyland B. <me@ow.swag.toys>
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

#include "draggable_region.hpp"
#include "supertux/debug.hpp"
#include "editor/editor.hpp"

DraggableRegion::DraggableRegion(Color color) :
  MovingObject(),
  m_color(std::move(color))
{}


DraggableRegion::DraggableRegion(const ReaderMapping &reader) :
  MovingObject(reader),
  m_color()
{}

DraggableRegion::DraggableRegion(Color color, const ReaderMapping &reader) :
  MovingObject(reader),
  m_color(std::move(color))
{
}

void
DraggableRegion::draw_draggable_box(DrawingContext& context)
{
  if (!Editor::is_active() && !g_debug.show_collision_rects)
    return;

  Rectf& box = m_col.m_bbox;
  if (Editor::current()->get_draggables_visible())
  {
    context.color().draw_filled_rect(box, m_color, 0.0f, LAYER_OBJECTS);
    return;
  }

  context.color().draw_line(
        { box.get_left(), box.get_top() },
        { box.get_right(), box.get_top() }, m_color, LAYER_OBJECTS);
  context.color().draw_line(
        { box.get_left(), box.get_top() },
        { box.get_left(), box.get_bottom() }, m_color, LAYER_OBJECTS);
  context.color().draw_line(
        { box.get_left(), box.get_bottom() },
        { box.get_right(), box.get_bottom() }, m_color, LAYER_OBJECTS);
  context.color().draw_line(
        { box.get_right(), box.get_top() },
        { box.get_right(), box.get_bottom() }, m_color, LAYER_OBJECTS);
}
