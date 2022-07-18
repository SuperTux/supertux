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

#include "gui/item_intfield.hpp"

ItemIntField::ItemIntField(const std::string& text_, int* input_, int id_) :
  ItemTextField(text_, new std::string, id_),
  number(input_),
  m_input(std::to_string(*input_))
{
  change_input(m_input);
}

ItemIntField::~ItemIntField()
{
  delete input;
}

bool
ItemIntField::custom_event(const SDL_Event& ev)
{
  if (ev.type == SDL_TEXTINPUT) {
    std::string txt = ev.text.text;
    for (auto& c : txt)
    {
      add_char(c);
    }
  }
  else if (ev.type == SDL_KEYDOWN)
  {
    if (ev.key.keysym.sym == SDLK_DELETE) // Delete back
    {
      if (!input->empty() && m_cursor_left_offset > 0)
      {
        *input = input->substr(0, input->size() - m_cursor_left_offset) +
          input->substr(input->size() - m_cursor_left_offset + 1);
        m_cursor_left_offset--;

        if (!input->empty())
        {
          int new_number = std::stoi(*input);
          *number = new_number;
        }
        else
        {
          *number = 0;
        }
      }
      else
      {
        invalid_remove();
      }
    }
  }
  return false;
}

void
ItemIntField::process_action(const MenuAction& action)
{
  if (action == MenuAction::REMOVE) // Delete front (backspace)
  {
    if (!input->empty() && m_cursor_left_offset < static_cast<int>(input->size()))
    {
      *input = input->substr(0, static_cast<int>(input->size()) - m_cursor_left_offset - 1) +
        input->substr(input->size() - m_cursor_left_offset);

      if (!input->empty())
      {
        int new_number = std::stoi(*input);
        *number = new_number;
      }
      else
      {
        *number = 0;
      }
    }
    else
    {
      invalid_remove();
    }
  }
  else if (action == MenuAction::LEFT) // Left
  {
    if (m_cursor_left_offset >= static_cast<int>(input->size()))
      return;

    m_cursor_left_offset++;
  }
  else if (action == MenuAction::RIGHT) // Right
  {
    if (m_cursor_left_offset <= 0)
      return;

    m_cursor_left_offset--;
  }
}

void
ItemIntField::add_char(char c)
{
  if (c == '-')
  {
    if (!input->empty() && *input != "0")
    {
      *number *= -1;
      *input = std::to_string(*number);
    }
    else
    {
      *input = "-";
    }
  }

  if (c < '0' || c > '9')
    return;

  *input = input->substr(0, input->size() - m_cursor_left_offset) + c +
    input->substr(input->size() - m_cursor_left_offset);
  try
  {
    int new_number = std::stoi(*input);
    *number = new_number;
  }
  catch (...)
  {
    *input = std::to_string(*number);
  }
}

/* EOF */
