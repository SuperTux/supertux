//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
//                2023 Vankata453
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

#include "interface/control_textbox_int.hpp"

ControlTextboxInt::ControlTextboxInt() :
  ControlTextbox(false),
  m_validate_int(),
  m_value(nullptr)
{
}

void
ControlTextboxInt::set_value(int value)
{
  if (m_value)
    *m_value = value;

  m_internal_string_backup = std::to_string(*m_value);
  put_text(m_internal_string_backup);
}

void
ControlTextboxInt::bind_value(int* value)
{
  m_value = value;

  if (m_value)
  {
    m_internal_string_backup = std::to_string(*m_value);
    put_text(m_internal_string_backup);
  }
  else
  {
    m_charlist.clear();
    m_charlist.push_back({});
  }

  set_caret_pos(0);
  set_secondary_caret_pos(0);
}

bool
ControlTextboxInt::validate_value()
{
  if (!ControlTextbox::validate_value())
    return false;

  const std::string contents = get_contents();

  int temp;
  try
  {
    temp = std::stoi(contents);
    if (contents != std::to_string(temp)) // Ensure full string has been converted
      return false;
  }
  catch (const std::exception&)
  {
    return false;
  }

  if (m_validate_int && !m_validate_int(this, temp))
    return false;

  if (m_value)
    *m_value = temp;

  return true;
}

/* EOF */
