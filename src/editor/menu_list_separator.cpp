//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "editor/menu_list_separator.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"

MenuListSeparator::MenuListSeparator() :
  MenuListBaseItem()
{
  set_size(Sizef(DEFAULT_SIZE.width, 1.f));
}

void
MenuListSeparator::draw(DrawingContext& context)
{
  auto line_color = Color(0.5f, 0.5f, 0.5f);
  auto line_start = get_rect().p1() + Vector(0, get_rect().get_height() / 2.f);
  auto line_end = Vector(get_rect().p2().x, line_start.y);
  context.color().draw_line(line_start, line_end, line_color, LAYER_GUI);
}