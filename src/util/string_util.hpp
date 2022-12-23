//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_UTIL_STRING_UTIL_HPP
#define HEADER_SUPERTUX_UTIL_STRING_UTIL_HPP

#include <string>

class StringUtil final
{
public:
  static bool has_suffix(const std::string& data, const std::string& suffix);

  /** Compare two strings according to their numeric value, similar to
      what 'sort -n' does. */
  static bool numeric_less(const std::string& lhs, const std::string& rhs);

  static std::string tolower(const std::string& text);

  static std::string replace_all(const std::string& haystack,
                                 const std::string& needle,
                                 const std::string& replacement);
};

#endif

/* EOF */
