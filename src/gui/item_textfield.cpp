//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2022 Vankata453
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

#include "gui/item_textfield.hpp"

#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

ItemTextField::ItemTextField(const std::string& text_, std::string* input_, int id_) :
  MenuItem(text_, id_),
  input(input_),
  m_cursor_char('_'),
  m_cursor_char_width(static_cast<int>(Resources::normal_font->get_text_width(std::to_string(m_cursor_char)))),
  m_cursor_left_offset(0)
{
}

void
ItemTextField::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active)
{
  std::string r_input = *input;
  if (active)
  {
    r_input = r_input.substr(0, r_input.size() - m_cursor_left_offset) + m_cursor_char +
      r_input.substr(r_input.size() - m_cursor_left_offset);
  }
  context.color().draw_text(Resources::normal_font, r_input,
                            Vector(pos.x + static_cast<float>(menu_width) - 16.0f - static_cast<float>(active ? 0 : m_cursor_char_width),
                                   pos.y - Resources::normal_font->get_height() / 2.0f),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::field_color);
  context.color().draw_text(Resources::normal_font, get_text(),
                            Vector(pos.x + 16.0f,
                                   pos.y - static_cast<float>(Resources::normal_font->get_height()) / 2.0f),
                            ALIGN_LEFT, LAYER_GUI, active ? g_config->activetextcolor : get_color());
}

int
ItemTextField::get_width() const
{
  return static_cast<int>(Resources::normal_font->get_text_width(get_text()) + Resources::normal_font->get_text_width(*input) + 16.0f + static_cast<float>(m_cursor_char_width));
}

void
ItemTextField::event(const SDL_Event& ev)
{
  if (ev.type == SDL_TEXTINPUT)
  {
    *input = input->substr(0, input->size() - m_cursor_left_offset) + ev.text.text +
      input->substr(input->size() - m_cursor_left_offset);
  }
}

void
ItemTextField::process_action(const MenuAction& action)
{
  if (action == MenuAction::REMOVE) {
    if (!input->empty() && m_cursor_left_offset < input->size())
    {
      *input = input->substr(0, input->size() - m_cursor_left_offset - 1) +
        input->substr(input->size() - m_cursor_left_offset);
    }
    else
    {
      invalid_remove();
    }
  }
  else if (action == MenuAction::LEFT)
  {
    if (m_cursor_left_offset >= input->size())
      return;
    m_cursor_left_offset++;
  }
  else if (action == MenuAction::RIGHT)
  {
    if (m_cursor_left_offset <= 0)
      return;
    m_cursor_left_offset--;
  }
}

/* EOF */
