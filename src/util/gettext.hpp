//  SuperTux 
//  Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_UTIL_GETTEXT_HPP
#define HEADER_SUPERTUX_UTIL_GETTEXT_HPP

#include "tinygettext/tinygettext.hpp"

extern TinyGetText::DictionaryManager dictionary_manager;

static inline const char* _(const char* message)
{
  return dictionary_manager.get_dictionary().translate(message);
}

static inline std::string _(const std::string& message)
{
  return dictionary_manager.get_dictionary().translate(message);
}

static inline const char* N_(const char* id, const char* id2, int num)
{
  return dictionary_manager.get_dictionary().translate(id, id2, num).c_str();
}

#endif /* _LIBGETTEXT_H */
