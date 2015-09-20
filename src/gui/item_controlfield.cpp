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

#include "gui/item_controlfield.hpp"

#include <stdio.h>

#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

ItemControlField::ItemControlField(const std::string& text_, const std::string& input_, int _id) :
  MenuItem(text_, _id),
  input(input_)
{
}

void
ItemControlField::draw(DrawingContext& context, Vector pos, int menu_width, bool active) {
  context.draw_text(Resources::normal_font, input,
                    Vector(pos.x + menu_width - 16, pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::field_color);
  context.draw_text(Resources::normal_font, text,
                    Vector(pos.x + 16, pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_LEFT, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());
}

int
ItemControlField::get_width() const {
  return Resources::normal_font->get_text_width(text) + Resources::normal_font->get_text_width(input) + 16;
}

/* EOF */
