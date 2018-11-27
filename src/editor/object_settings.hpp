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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_SETTINGS_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_SETTINGS_HPP

#include <vector>

#include "editor/object_option.hpp"

class Color;

class ObjectSettings final
{
public:
  ObjectSettings(const std::string& name);

  const std::string& get_name() const { return m_name; }

  void copy_from(const ObjectSettings& other);

  void add_option(const ObjectOption& option);

  void add_bool(const std::string& text, bool* value_ptr,
                const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_float(const std::string& text, float* value_ptr,
                 const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_int(const std::string& text, int* value_ptr,
               const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_badguy(const std::string& text, std::string* value_ptr,
                  const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_color(const std::string& text, Color* value_ptr,
                 const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_remove(const std::string& text, int* value_ptr,
                  const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_script(const std::string& text, std::string* value_ptr,
                  const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);
  void add_text(const std::string& text, std::string* value_ptr,
                const std::string& key = {}, int flags = OPTION_ALLOW_EMPTY | OPTION_VISIBLE);

  const std::vector<ObjectOption>& get_options() const { return m_options; }

  /** Avoid using this one */
  std::vector<ObjectOption>& get_options_writable() { return m_options; }

  template <typename ...Args>
  void add(Args && ...args) {
    add_option(ObjectOption(std::forward<Args>(args)...));
  }

private:
  std::string m_name;
  std::vector<ObjectOption> m_options;
};

#endif

/* EOF */
