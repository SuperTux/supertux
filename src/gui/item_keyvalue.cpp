//  SuperTux
//  Copyright (C) 2017 christ2go <christian@hagemeier.ch>
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

#include "gui/item_keyvalue.hpp"

#include "gui/menu_action.hpp"
#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

void KeyValueLabel::draw(DrawingContext &context, const Vector & pos, int menu_width,
                         bool active) {
  std::string r_input = value;
  context.draw_text(
      Resources::normal_font, r_input,
      Vector(pos.x + menu_width - 16,
             pos.y - int(Resources::normal_font->get_height() / 2)),
      ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::field_color);
  context.draw_text(
      Resources::normal_font, text,
      Vector(pos.x + 16, pos.y - int(Resources::normal_font->get_height() / 2)),
      ALIGN_LEFT, LAYER_GUI,
      active ? ColorScheme::Menu::active_color : get_color());
}

int KeyValueLabel::get_width() const {
  return Resources::normal_font->get_text_width(value) +
         Resources::normal_font->get_text_width(text) + 16;
}

void KeyValueLabel::event(const SDL_Event &ev) {}
