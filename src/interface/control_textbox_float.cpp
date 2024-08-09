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

#include "interface/control_textbox_float.hpp"

static std::string format_float_string(std::string str)
{
  // Remove the trailing zeroes at the end of the decimal point...
  while (!str.empty() && str.at(str.size() - 1) == '0')
    str.pop_back();

  // ...but keep at least one number after the point.
  if (!str.empty() && str.at(str.size() - 1) == '.')
    str += "0";

  return str;
}


ControlTextboxFloat::ControlTextboxFloat() :
  ControlTextbox(false),
  m_validate_float(),
  m_value(nullptr)
{
}

void
ControlTextboxFloat::set_value(float value)
{
  if (m_value)
    *m_value = value;

  m_internal_string_backup = format_float_string(std::to_string(*m_value));
  put_text(m_internal_string_backup);
}

void
ControlTextboxFloat::bind_value(float* value)
{
  m_value = value;

  if (m_value)
  {
    m_internal_string_backup = format_float_string(std::to_string(*m_value));
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
ControlTextboxFloat::validate_value()
{
  if (!ControlTextbox::validate_value())
    return false;

  const std::string contents = get_contents();

  float temp;
  try
  {
    temp = std::stof(contents);
  }
  catch (const std::exception&)
  {
    return false;
  }

  if (m_validate_float && !m_validate_float(this, temp))
    return false;

  if (m_value)
    *m_value = temp;

  return true;
}

/* EOF */
