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

#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

ItemColorChannel::ItemColorChannel(float* input_, Color channel_, int id_) :
  MenuItem(std::to_string(*input_), id_),
  number(input_),
  flickw(static_cast<int>(Resources::normal_font->get_text_width("_"))),
  has_comma(true),
  channel(channel_)
{
  // removing all redundant zeros at the end
  for (auto i = m_text.end() - 1; i != m_text.begin(); --i) {
    char c = *i;
    if (c == '.') {
      m_text.resize(m_text.length() - 1);
      has_comma = false;
    }
    if (c != '0') {
      break;
    }
    m_text.resize(m_text.length() - 1);
  }
}

void
ItemColorChannel::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  MenuItem::draw(context, pos, menu_width, active);
  float lw = float(menu_width - 32) * (*number);
  context.color().draw_filled_rect(Rectf(pos + Vector(16, 6), pos + Vector(16 + lw, 16)),
                                     channel, 0.0f, LAYER_GUI-1);
}

int
ItemColorChannel::get_width() const {
  return static_cast<int>(Resources::normal_font->get_text_width(m_text) + 16 + static_cast<float>(flickw));
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
    if (!m_text.length()) {
      m_text = "0.";
    } else {
      m_text.push_back('.');
    }
    has_comma = true;
  }

  if (c < '0' || c > '9') {
    return;
  }

  m_text.push_back(c);
  *number = std::stof(m_text);

  if (*number < 0 || *number > 1) {
    remove_char();
  }
}

void
ItemColorChannel::remove_char() {
  unsigned char last_char;
  do {
    last_char = *(--m_text.end());
    m_text.resize(m_text.length() - 1);
    if (m_text.length() == 0) {
      break;
    }
    if (last_char == '.') {
      has_comma = false;
    }
  } while ( (last_char & 128) && !(last_char & 64) );
  if (m_text.length() && m_text != "-") {
    *number = std::stof(m_text);
  } else {
    *number = 0;
  }
}

void
ItemColorChannel::process_action(const MenuAction& action) {
  if (action == MENU_ACTION_REMOVE && m_text.length()) {
    remove_char();
  }
}

Color
ItemColorChannel::get_color() const {
  return channel;
}

/* EOF */
