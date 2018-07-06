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

ObjectOption::ObjectOption(MenuItemKind ip_type, const std::string& text_, void* ip,
                           const std::string& key_, int flags_) :
  type(ip_type),
  text(text_),
  option(ip),
  key(key_),
  flags(flags_),
  select()
{
}

const std::string
ObjectOption::to_string() const {
  switch (type) {
    case MN_TEXTFIELD:
      return *((std::string*)(option));
    case MN_NUMFIELD:
      return std::to_string(*((float*)(option)));
    case MN_INTFIELD:
      return std::to_string(*((int*)(option)));
    case MN_TOGGLE:
      return (*((bool*)(option))) ? _("true") : _("false");
    case MN_STRINGSELECT: {
      auto selected_id = (int*)option;
      if ( *selected_id >= int(select.size()) || *selected_id < 0 ) {
        return _("invalid"); //Test whether the selected ID is valid
      } else {
        return select[*selected_id];
      }
    }
    case MN_BADGUYSELECT:
      return std::to_string(((std::vector<std::string>*)option)->size());
    case MN_COLOR:
      return ((Color*)option)->to_string();
    case MN_SCRIPT:
      if (((std::string*)option)->length()) {
        return "...";
      }
      return "";
    case MN_FILE:
      return *((std::string*)(option));
    default:
      return _("Unknown");
  }
}

/* EOF */
