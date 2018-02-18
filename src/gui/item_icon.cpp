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

#include "gui/item_icon.hpp"
#include "video/drawing_context.hpp"

ItemIcon::ItemIcon(const std::string& text_, int _id, const SurfacePtr& icon) :
  MenuItem(text_, _id),
  icon(icon)
{
}

void
ItemIcon::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active)
{
  MenuItem::draw(context, pos, menu_width, active);
  context.draw_surface_part(icon, Rectf(Vector(0, 0), icon->get_size()),
                            Rectf(pos + Vector(10, -10), pos + Vector(30, 10)), LAYER_GUI);
}

/* EOF */
