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

ItemColorChannel::ItemColorChannel(float* input, Color channel, int id) :
  MenuItem(std::to_string(*input), id),
  m_number(input),
  m_flickw(static_cast<int>(Resources::normal_font->get_text_width("_"))),
  m_has_comma(true),
  m_channel(channel)
{
  // removing all redundant zeros at the end
  std::string text = get_text();
  for (auto i = text.end() - 1; i != text.begin(); --i) {
    char c = *i;
    if (c == '.') {
      text.resize(text.length() - 1);
      m_has_comma = false;
    }
    if (c != '0') {
      break;
    }
    text.resize(text.length() - 1);
  }
  set_text(text);
}

void
ItemColorChannel::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active)
{
  MenuItem::draw(context, pos, menu_width, active);
  float lw = float(menu_width - 32) * (*m_number);
  context.color().draw_filled_rect(Rectf(pos + Vector(16, 6), pos + Vector(16 + lw, 16)),
                                   m_channel, 0.0f, LAYER_GUI-1);
}

int
ItemColorChannel::get_width() const
{
  return static_cast<int>(Resources::normal_font->get_text_width(get_text()) + 16 + static_cast<float>(m_flickw));
}

void
ItemColorChannel::event(const SDL_Event& ev)
{
  if (ev.type == SDL_TEXTINPUT) {
    std::string txt = ev.text.text;
    for (auto& c : txt) {
      add_char(c);
    }
  }
}

void
ItemColorChannel::add_char(char c)
{
  std::string text = get_text();

  if (!m_has_comma && (c == '.' || c == ',')) {
    if (!text.length()) {
      text = "0.";
    } else {
      text.push_back('.');
    }
    set_text(text);
    m_has_comma = true;
  }

  if (c < '0' || c > '9') {
    return;
  }

  text.push_back(c);
  *m_number = std::stof(text);

  if (*m_number < 0 || *m_number > 1) {
    remove_char();
  }

  set_text(text);
}

void
ItemColorChannel::remove_char()
{
  std::string text = get_text();

  unsigned char last_char;

  do {
    last_char = *(--text.end());
    text.resize(text.length() - 1);
    if (text.length() == 0) {
      break;
    }
    if (last_char == '.') {
      m_has_comma = false;
    }
  } while ( (last_char & 128) && !(last_char & 64) );

  if (text.length() && text != "-") {
    *m_number = std::stof(text);
  } else {
    *m_number = 0;
  }

  set_text(text);
}

void
ItemColorChannel::process_action(const MenuAction& action)
{
  if (action == MENU_ACTION_REMOVE && get_text().length()) {
    remove_char();
  }
}

Color
ItemColorChannel::get_color() const
{
  return m_channel;
}

/* EOF */
