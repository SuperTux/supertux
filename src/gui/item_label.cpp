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

#include "gui/item_label.hpp"

#include <stdio.h>

#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

ItemLabel::ItemLabel(const std::string& text_) :
  MenuItem(text_)
{
}

void
ItemLabel::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  context.draw_text(Resources::big_font, text,
                    Vector( pos.x + menu_width/2 , pos.y - int(Resources::big_font->get_height())/2 ),
                    ALIGN_CENTER, LAYER_GUI, get_color());
}

Color
ItemLabel::get_color() const {
  return ColorScheme::Menu::label_color;
}

int
ItemLabel::get_width() const {
  return Resources::big_font->get_text_width(text) + 16;
}

/* EOF */
