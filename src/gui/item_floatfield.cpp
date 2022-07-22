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

#include "gui/item_floatfield.hpp"

ItemFloatField::ItemFloatField(const std::string& text_, float* input_, int id_) :
  ItemTextField(text_, new std::string, id_),
  number(input_),
  m_input(std::to_string(*input_)),
  m_has_comma(true)
{
  change_input(m_input);

  // Removing all redundant zeros at the end.
  for (auto i = input->end() - 1; i != input->begin(); --i) {
    char c = *i;
    if (c == '.')
    {
      input->resize(input->size() - 1);
      m_has_comma = false;
    }
    if (c != '0') break;
    input->resize(input->size() - 1);
  }
}

ItemFloatField::~ItemFloatField()
{
  delete input;
}

void
ItemFloatField::add_char(char c, const int index)
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
  else if (!m_has_comma && (c == '.' || c == ','))
  {
    update_undo();
    if (index == static_cast<int>(input->size()))
    {
      *input = "0." + *input;
    }
    else
    {
      *input = input->substr(0, input->size() - index) + '.' +
        input->substr(input->size() - index);
    }
    m_has_comma = true;
  }

  if (c >= '0' && c <= '9')
  {
    update_undo();
    *input = input->substr(0, input->size() - index) + c +
      input->substr(input->size() - index);
  }
  on_input_update();
}

void
ItemFloatField::on_input_update()
{
  if (input->empty() || *input == "-")
  {
    m_has_comma = false;
    *number = 0;
    return;
  }

  m_has_comma = input->find(".") != std::string::npos || input->find(",") != std::string::npos;

  try
  {
    *number = std::stof(*input);
  }
  catch (...)
  {
    *input = std::to_string(*number);
  }
}

// Text manipulation and navigation functions

void
ItemFloatField::insert_at(const std::string& text, const int index)
{
  for (auto& c : text)
  {
    add_char(c, index);
  }
}

/* EOF */
