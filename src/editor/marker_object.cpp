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

#include "editor/marker_object.hpp"

#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

MarkerObject::MarkerObject (const Vector& pos)
  : arrow_surface(Surface::from_file("images/engine/editor/resize_arrow.png"))
{
  m_col.m_bbox.set_p1(pos);
  m_col.m_bbox.set_size(16, 16);
}

MarkerObject::MarkerObject ():
  MarkerObject(Vector(0, 0))
{
}

MarkerObject::~MarkerObject()
{
  arrow_surface.reset();
}

int
MarkerObject::get_rotation() const
{
  Vector dir = get_point_vector();
  if (dir.x == 0 && dir.y == 0)
    return 0;

  return static_cast<int>(std::round(glm::degrees(std::atan2(dir.y, dir.x)))) + 90;
}

void
MarkerObject::draw(DrawingContext& context)
{
  Vector dir = get_point_vector();
  if (dir.x == 0 && dir.y == 0) {
    if (hide_if_no_offset())
      return;
    context.color().draw_filled_rect(m_col.m_bbox, Color(1, 1, 1, 0.5), 7.5, LAYER_GUI-20);
  } else {
    context.color().draw_surface(arrow_surface, m_col.m_bbox.get_middle() - Vector(7, 7),
                                 get_rotation(), Color::WHITE, Blend::BLEND, get_layer());
  }
}
