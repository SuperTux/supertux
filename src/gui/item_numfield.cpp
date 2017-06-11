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

#include "gui/item_numfield.hpp"

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

ItemNumField::ItemNumField(const std::string& text_, float* input_, int id_) :
  MenuItem(text_, id_),
  number(input_),
  input(std::to_string(*input_)),
  flickw(Resources::normal_font->get_text_width("_")),
  has_comma(true)
{
  // removing all redundant zeros at the end
  for (auto i = input.end() - 1; i != input.begin(); --i) {
    char c = *i;
    if (c == '.') {
      input.resize(input.length() - 1);
      has_comma = false;
    }
    if (c != '0') {
      break;
    }
    input.resize(input.length() - 1);
  }
}

void
ItemNumField::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
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
ItemNumField::get_width() const {
  return Resources::normal_font->get_text_width(text) + Resources::normal_font->get_text_width(input) + 16 + flickw;
}

void
ItemNumField::event(const SDL_Event& ev) {
  if (ev.type == SDL_TEXTINPUT) {
    std::string txt = ev.text.text;
    for (auto i = txt.begin(); i != txt.end(); ++i) {
      add_char(*i);
    }
  }
}

void
ItemNumField::add_char(char c) {
  if (c == '-') {
    if (input.length() && input != "0") {
      *number *= -1;
      if (*input.begin() == '-') {
        input.erase(input.begin());
      } else {
        input.insert(input.begin(),'-');
      }
    } else {
      input = "-";
    }
  } else if (!has_comma && (c == '.' || c == ',')) {
    if (!input.length()) {
      input = "0.";
    } else {
      input.push_back('.');
    }
    has_comma = true;
  }

  if (c < '0' || c > '9') {
    return;
  }

  input.push_back(c);
  try {
    float new_number = std::stof(input);
    *number = new_number;
  } catch (...) {
    input = std::to_string(*number);
  }
}

void
ItemNumField::process_action(const MenuAction& action) {
  if (action == MENU_ACTION_REMOVE && input.length()) {
    unsigned char last_char;
    do {
      last_char = *(--input.end());
      input.resize(input.length() - 1);
      if (input.length() == 0) {
        break;
      }
      if (last_char == '.') {
        has_comma = false;
      }
    } while ( (last_char & 128) && !(last_char & 64) );
    if (input.length() && input != "-") {
      *number = std::stof(input);
    } else {
      *number = 0;
    }
  }
}
