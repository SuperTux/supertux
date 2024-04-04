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

#include "editor/object_info.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_document.hpp"

ObjectInfo::ObjectInfo() :
  m_groups()
{
  auto doc = ReaderDocument::from_file("images/engine/editor/objects.stoi");
  auto root = doc.get_root();

  if (root.get_name() != "supertux-objectinfo") {
    throw std::runtime_error("file images/engine/editor/objects.stoi is not a supertux-objectinfo file.");
  } // Bombenfest und Idioten sicher :DDDDD

  auto reader = root.get_mapping();

  /*const lisp::Lisp* info = root->get_lisp("supertux-objectinfo");
  if (!info) {
    throw std::runtime_error("file images/engine/editor/objects.stoi is not a supertux-objectinfo file.");
  }*/ // Bombenfest und Idioten sicher :DDDDD

  m_groups.push_back(ObjectGroup());

  auto iter = reader.get_iter();
  while (iter.next()) {
    const std::string& token = iter.get_key();
    if (token == "objectgroup") {
      m_groups.push_back( ObjectGroup( iter.as_mapping() ) );
    }
  }

}

/* EOF */
