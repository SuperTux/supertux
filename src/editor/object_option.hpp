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
    ObjectOption(MenuItemKind ip_type, std::string text_, void* ip,
                 std::string key_ = "", bool visible_ = true, bool allow_empty_ = true);
    ~ObjectOption();

    MenuItemKind type;
    std::string text;
    void* option;
    std::string key;
    // Visible in object options
    bool visible;
    // Allow empty value?
    bool allow_empty;
    bool is_savable() const {
      return !key.empty();
    }

    std::vector<std::string> select;

    ObjectOption(const ObjectOption& blb) :
      type(blb.type),
      text(blb.text),
      option(blb.option),
      key(blb.key),
      visible(blb.visible),
      allow_empty(blb.allow_empty),
      select(blb.select)
    { /* blb-ost */ }

    ObjectOption& operator=(const ObjectOption& blb)
    {
      type = blb.type;
      text = blb.text;
      option = blb.option;
      select = blb.select;
      key = blb.key;
      visible = blb.visible;
      allow_empty = blb.allow_empty;
      return *this;
    }

    const std::string to_string();
};

#endif // HEADER_SUPERTUX_EDITOR_OBJECT_OPTION_HPP

/* EOF */
