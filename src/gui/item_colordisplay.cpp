//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "gui/item_colordisplay.hpp"

#include "video/drawing_context.hpp"

ItemColorDisplay::ItemColorDisplay(Color* color, int id_) :
  MenuItem("", id_),
  old_color(*color),
  new_color(color)
{
}

void
ItemColorDisplay::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  float m = menu_width / 2.f;
  context.color().draw_filled_rect(Rectf(pos + Vector(16, -8), pos + Vector(m, 8)),
                           old_color, 0.0f, LAYER_GUI-1);
  context.color().draw_filled_rect(Rectf(pos + Vector(m, -8), pos + Vector(menu_width-16, 8)),
                           *new_color, 0.0f, LAYER_GUI-1);
}

int
ItemColorDisplay::get_width() const {
  return 0;
}

/* EOF */
