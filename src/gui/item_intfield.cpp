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

void
ItemIntField::add_char(char c, const int index)
{
  if (c == '-')
  {
    update_undo();
    if (!input->empty() && *input != "0")
    {
      *number *= -1;
      if (*input->begin() == '-')
      {
        if (m_cursor_left_offset == static_cast<int>(input->size())) m_cursor_left_offset--;
        input->erase(input->begin());
      }
      else
      {
        input->insert(input->begin(), '-');
      }
    }
    else
    {
      *input = "-";
    }
  }

  if (c < '0' || c > '9')
    return;

  update_undo();
  *input = input->substr(0, input->size() - index) + c +
    input->substr(input->size() - index);
  on_input_update();
}

void
ItemIntField::on_input_update()
{
  if (input->empty())
  {
    *number = 0;
    return;
  }

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

// Text manipulation and navigation functions

void
ItemIntField::insert_at(const std::string& text, const int index)
{
  for (auto& c : text)
  {
    add_char(c, index);
  }
}

/* EOF */
