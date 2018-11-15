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

// ObjectOption bitfield flags
enum ObjectOptionFlags {
  OPTION_ALLOW_EMPTY = (1 << 0),
  OPTION_VISIBLE = (1 << 1)
};

class ObjectOption final
{
public:
  ObjectOption(MenuItemKind ip_type, const std::string& text, void* ip,
               const std::string& key = std::string(), int flags = (OPTION_ALLOW_EMPTY | OPTION_VISIBLE));
  ObjectOption(const ObjectOption& other) = default;
  ObjectOption& operator=(const ObjectOption& other) = default;

  bool is_savable() const { return !m_key.empty(); }

  const std::string to_string() const;

  void add_select(const std::string& text);

public:
  MenuItemKind m_type;
  std::string m_text;
  void* m_option;
  std::string m_key;
  int m_flags;
  std::vector<std::string> m_select;
};

#endif

/* EOF */
