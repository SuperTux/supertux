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

#include "gui/item_intfield.hpp"

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

ItemIntField::ItemIntField(const std::string& text_, int* input_, int id_) :
  MenuItem(text_, id_),
  number(input_),
  input(std::to_string(*input_)),
  flickw(0)
{
  flickw = Resources::normal_font->get_text_width("_");
}

void
ItemIntField::draw(DrawingContext& context, Vector pos, int menu_width, bool active) {
  std::string r_input = input;
  bool fl = active && (int(real_time*2)%2);
  if ( fl ) {
    r_input += "_";
  }
  context.draw_text(Resources::normal_font, r_input,
                    Vector(pos.x + menu_width - 16 - (fl ? 0 : flickw), pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::field_color);
  context.draw_text(Resources::normal_font, text,
                    Vector(pos.x + 16, pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_LEFT, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());
}

int
ItemIntField::get_width() const {
  return Resources::normal_font->get_text_width(text) + Resources::normal_font->get_text_width(input) + 16 + flickw;
}

void
ItemIntField::event(const SDL_Event& ev) {
  if (ev.type == SDL_TEXTINPUT) {
    std::string txt = ev.text.text;
    for (auto i = txt.begin(); i != txt.end(); ++i) {
      add_char(*i);
    }
  }
}

void
ItemIntField::add_char(char c) {
  if (c == '-') {
    if (input.length() && input != "0") {
      *number *= -1;
      input = std::to_string(*number);
    } else {
      input = "-";
    }
  }

  if (c < '0' || c > '9') {
    return;
  }

  input.push_back(c);
  *number = std::stoi(input);
}

void
ItemIntField::process_action(MenuAction action) {
  if (action == MENU_ACTION_REMOVE && input.length()) {
    unsigned char last_char;
    do {
      last_char = *(--input.end());
      input.resize(input.length() - 1);
      if (input.length() == 0) {
        break;
      }
    } while ( (last_char & 128) && !(last_char & 64) );
    if (input.length()) {
      *number = std::stoi(input);
    } else {
      *number = 0;
    }
  }
}
