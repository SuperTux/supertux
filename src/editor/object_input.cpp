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

#include "editor/object_input.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_document.hpp"

ObjectInput::ObjectInput() :
  groups()
{
  groups.clear();

  //lisp::Parser parser;
  //const lisp::Lisp* root = parser.parse("images/engine/editor/objects.stoi");

  auto doc = ReaderDocument::parse("images/engine/editor/objects.stoi");
  auto root = doc.get_root();

  if(root.get_name() != "supertux-objectinfo") {
    throw std::runtime_error("file images/engine/editor/objects.stoi is not a supertux-objectinfo file.");
  } // Bombenfest und Idioten sicher :DDDDD

  auto reader = root.get_mapping();

  /*const lisp::Lisp* info = root->get_lisp("supertux-objectinfo");
  if(!info) {
    throw std::runtime_error("file images/engine/editor/objects.stoi is not a supertux-objectinfo file.");
  }*/ // Bombenfest und Idioten sicher :DDDDD

  auto iter = reader.get_iter();
  while(iter.next()) {
    const std::string& token = iter.get_key();
    if (token == "objectgroup") {
      groups.push_back( ObjectGroup( iter.as_mapping() ) );
    }
  }

}

ObjectInput::~ObjectInput() {

}

/* EOF */
