/*
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */
#ifndef _LIBGETTEXT_H
#define _LIBGETTEXT_H

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
