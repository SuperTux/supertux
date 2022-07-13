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

#include <boost/algorithm/string.hpp>

#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

ItemTextField::ItemTextField(const std::string& text_, std::string* input_, int id_) :
  MenuItem(text_, id_),
  input(input_),
  m_input_undo(""),
  m_input_redo(""),
  m_cursor_char('|'),
  m_cursor_char_str(std::string(1, m_cursor_char)),
  m_cursor_char_width(static_cast<int>(Resources::normal_font->get_text_width(m_cursor_char_str))),
  m_cursor_left_offset(0)
{
}

void
ItemTextField::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active)
{
  context.color().draw_text(Resources::normal_font, *input,
                            Vector(pos.x + static_cast<float>(menu_width) - 16.0f,
                                   pos.y - Resources::normal_font->get_height() / 2.0f),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::field_color);
  if (active && (int(g_real_time * 2) % 2))
  {
    // Draw text cursor.
    context.color().draw_text(Resources::normal_font, m_cursor_char_str,
                              Vector(pos.x + static_cast<float>(menu_width) - 18.0f -
                                       Resources::normal_font->get_text_width(input->substr(input->size() - m_cursor_left_offset)),
                                     pos.y - Resources::normal_font->get_height() / 2.0f),
                              ALIGN_LEFT, LAYER_GUI, Color::CYAN);
  }
  context.color().draw_text(Resources::normal_font, get_text(),
                            Vector(pos.x + 16.0f,
                                   pos.y - static_cast<float>(Resources::normal_font->get_height()) / 2.0f),
                            ALIGN_LEFT, LAYER_GUI, active ? g_config->activetextcolor : get_color());
}

void
ItemTextField::set_input_text(const std::string& text)
{
  *input = text;
  m_input_undo.clear();
  m_input_redo.clear();
}

int
ItemTextField::get_width() const
{
  return static_cast<int>(Resources::normal_font->get_text_width(get_text()) + Resources::normal_font->get_text_width(*input) + 16.0f + static_cast<float>(m_cursor_char_width));
}

void
ItemTextField::event(const SDL_Event& ev)
{
  if (!custom_event(ev)) return; // If a given custom event function returns "false", do not execute the other base events.

  if (ev.type == SDL_TEXTINPUT) // Text input
  {
    m_input_undo = *input;
    m_input_redo.clear();
    *input = input->substr(0, input->size() - m_cursor_left_offset) + ev.text.text +
      input->substr(input->size() - m_cursor_left_offset);
  }
  else if (ev.type == SDL_KEYDOWN)
  {
    if (ev.key.keysym.sym == SDLK_DELETE) // Delete back
    {
      if (!input->empty() && m_cursor_left_offset > 0)
      {
        m_input_undo = *input;
        m_input_redo.clear();
        unsigned char next_char = '\0';
        do
        {
          next_char = m_cursor_left_offset == 1 ? next_char : input->at(input->size() - m_cursor_left_offset + 1);
          *input = input->substr(0, input->size() - m_cursor_left_offset) +
            input->substr(input->size() - m_cursor_left_offset + 1);
          m_cursor_left_offset--;
          if (input->empty() || m_cursor_left_offset <= 0) break;
        } while ((next_char & 128) && !(next_char & 64));
      }
      else
      {
        invalid_remove();
      }
    }
    else if (ev.key.keysym.sym == SDLK_HOME) // Home: go to beginning of text
    {
      m_cursor_left_offset = input->size();
    }
    else if (ev.key.keysym.sym == SDLK_END) // End: go to end of text
    {
      m_cursor_left_offset = 0;
    }
    else if (SDL_GetModState() & KMOD_CTRL) //Commands which require CTRL
    {
      if (ev.key.keysym.sym == SDLK_v) // Paste
      {
        m_input_undo = *input;
        m_input_redo.clear();

        char* clipboard_content = SDL_GetClipboardText();
        std::string clipboard_text = std::string(clipboard_content);
        SDL_free(clipboard_content);
        boost::replace_all(clipboard_text, "\n", " "); // Replace any newlines with spaces.

        if (clipboard_text.empty()) return;
        *input = input->substr(0, input->size() - m_cursor_left_offset) + clipboard_text +
          input->substr(input->size() - m_cursor_left_offset);
      }
      else if (ev.key.keysym.sym == SDLK_z) // Undo
      {
        if (m_input_undo.empty()) return;
        m_input_redo = *input;
        *input = m_input_undo;
        m_input_undo.clear();
      }
      else if (ev.key.keysym.sym == SDLK_y) // Redo
      {
        if (m_input_redo.empty()) return;
        m_input_undo = *input;
        *input = m_input_redo;
        m_input_redo.clear();
      }
    }
  }
}

void
ItemTextField::process_action(const MenuAction& action)
{
  if (action == MenuAction::REMOVE)
  {
    if (!input->empty() && m_cursor_left_offset < static_cast<int>(input->size()))
    {
      unsigned char last_char;
      do
      {
        const int index = input->size() - m_cursor_left_offset - 1;
        last_char = input->at(index);
        *input = input->substr(0, index) +
          input->substr(input->size() - m_cursor_left_offset);
        if (input->empty() || m_cursor_left_offset >= static_cast<int>(input->size())) break;
      } while ((last_char & 128) && !(last_char & 64));
    }
    else
    {
      invalid_remove();
    }
  }
  else if (action == MenuAction::LEFT)
  {
    if (m_cursor_left_offset >= static_cast<int>(input->size()))
      return;

    unsigned char last_char;
    do
    {
      last_char = input->at(input->size() - m_cursor_left_offset - 1);
      m_cursor_left_offset++;
      if (m_cursor_left_offset >= static_cast<int>(input->size())) break;
    } while ((last_char & 128) && !(last_char & 64));
  }
  else if (action == MenuAction::RIGHT)
  {
    if (m_cursor_left_offset <= 0)
      return;

    unsigned char next_char = '\0';
    do
    {
      next_char = m_cursor_left_offset == 1 ? next_char : input->at(input->size() - m_cursor_left_offset + 1);
      m_cursor_left_offset--;
      if (m_cursor_left_offset <= 0) break;
    } while ((next_char & 128) && !(next_char & 64));
  }
}

/* EOF */
