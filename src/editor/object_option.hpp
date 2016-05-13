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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_OPTION_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_OPTION_HPP

#include <string>
#include <vector>

#include "gui/menu_action.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

class ObjectOption
{
  public:
    ObjectOption(MenuItemKind ip_type, std::string text_, void* ip);
    ~ObjectOption();

    MenuItemKind type;
    std::string text;
    void* option;

    std::vector<std::string> select;

    ObjectOption(const ObjectOption& blb) :
      type(blb.type),
      text(blb.text),
      option(blb.option),
      select(blb.select)
    { /* blb-ost */ }

    ObjectOption& operator=(const ObjectOption& blb)
    {
      type = blb.type;
      text = blb.text;
      option = blb.option;
      select = blb.select;
      return *this;
    }

    const std::string to_string() {
      switch (type) {
        case MN_TEXTFIELD:
          return *((std::string*)(option));
        case MN_NUMFIELD:
          return std::to_string(*((float*)(option)));
        case MN_INTFIELD:
          return std::to_string(*((int*)(option)));
        case MN_TOGGLE:
          return (*((bool*)(option))) ? _("true") : _("false");
        case MN_STRINGSELECT:
          return select[*((int*)(option))];
        case MN_BADGUYSELECT:
          return std::to_string(((std::vector<std::string>*)option)->size());
        case MN_COLOR:
          return std::to_string(((Color*)option)->red) + " "
                 + std::to_string(((Color*)option)->green) + " "
                 + std::to_string(((Color*)option)->blue);
        case MN_SCRIPT:
          if (((std::string*)option)->length()) {
            return "...";
          }
          return "";
        default:
          return _("Unknown");
      }
      return "";
    }
};

#endif // HEADER_SUPERTUX_EDITOR_OBJECT_OPTION_HPP

/* EOF */
