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

#include "gui/item_colorchannel.hpp"

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

ItemColorChannel::ItemColorChannel(float* input_, Color channel_, int id_) :
  MenuItem(std::to_string(*input_), id_),
  number(input_),
  flickw(0),
  has_comma(true),
  channel(channel_)
{
  flickw = Resources::normal_font->get_text_width("_");

  // removing all redundant zeros at the end
  for (auto i = text.end() - 1; i != text.begin(); --i) {
    char c = *i;
    if (c == '.') {
      text.resize(text.length() - 1);
      has_comma = false;
    }
    if (c != '0') {
      break;
    }
    text.resize(text.length() - 1);
  }
}

void
ItemColorChannel::draw(DrawingContext& context, Vector pos, int menu_width, bool active) {
  MenuItem::draw(context, pos, menu_width, active);
  float lw = float(menu_width - 32) * (*number);
  context.draw_filled_rect(Rectf(pos + Vector(16, 6), pos + Vector(16 + lw, 16)),
                           channel, 0.0f, LAYER_GUI-1);
}

int
ItemColorChannel::get_width() const {
  return Resources::normal_font->get_text_width(text) + 16 + flickw;
}

void
ItemColorChannel::event(const SDL_Event& ev) {
  if (ev.type == SDL_TEXTINPUT) {
    std::string txt = ev.text.text;
    for (auto& c : txt) {
      add_char(c);
    }
  }
}

void
ItemColorChannel::add_char(char c) {
  if (!has_comma && (c == '.' || c == ',')) {
    if (!text.length()) {
      text = "0.";
    } else {
      text.push_back('.');
    }
    has_comma = true;
  }

  if (c < '0' || c > '9') {
    return;
  }

  text.push_back(c);
  *number = std::stof(text);

  if (*number < 0 || *number > 1) {
    remove_char();
  }
}

void
ItemColorChannel::remove_char() {
  unsigned char last_char;
  do {
    last_char = *(--text.end());
    text.resize(text.length() - 1);
    if (text.length() == 0) {
      break;
    }
    if (last_char == '.') {
      has_comma = false;
    }
  } while ( (last_char & 128) && !(last_char & 64) );
  if (text.length() && text != "-") {
    *number = std::stof(text);
  } else {
    *number = 0;
  }
}

void
ItemColorChannel::process_action(MenuAction action) {
  if (action == MENU_ACTION_REMOVE && text.length()) {
    remove_char();
  }
}

Color
ItemColorChannel::get_color() const {
  return channel;
}
