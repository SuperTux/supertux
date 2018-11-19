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

#include "editor/object_option.hpp"

#include <string>
#include <vector>

#include "util/gettext.hpp"
#include "video/color.hpp"

ObjectOption::ObjectOption(MenuItemKind ip_type, const std::string& text, void* ip,
                           const std::string& key, int flags) :
  m_type(ip_type),
  m_text(text),
  m_option(ip),
  m_key(key),
  m_flags(flags),
  m_select()
{
}

std::string
ObjectOption::to_string() const
{
  switch (m_type)
  {
    case MN_TEXTFIELD:
      return *(static_cast<std::string*>(m_option));

    case MN_NUMFIELD:
      return std::to_string(*(static_cast<float*>(m_option)));

    case MN_INTFIELD:
      return std::to_string(*(static_cast<int*>(m_option)));

    case MN_TOGGLE:
      return (*(static_cast<bool*>(m_option))) ? _("true") : _("false");

    case MN_STRINGSELECT: {
      int* selected_id = static_cast<int*>(m_option);
      if ( *selected_id >= int(m_select.size()) || *selected_id < 0 ) {
        return _("invalid"); //Test whether the selected ID is valid
      } else {
        return m_select[*selected_id];
      }
    }

    case MN_BADGUYSELECT:
      return std::to_string((static_cast<std::vector<std::string>*>(m_option))->size());

    case MN_COLOR:
      return (static_cast<Color*>(m_option))->to_string();

    case MN_SCRIPT:
      if ((static_cast<std::string*>(m_option))->length()) {
        return "...";
      }
      return "";

    case MN_FILE:
      return *(static_cast<std::string*>(m_option));

    default:
      return _("Unknown");
  }
}

void
ObjectOption::add_select(const std::string& text)
{
  m_select.push_back(text);
}

/* EOF */
