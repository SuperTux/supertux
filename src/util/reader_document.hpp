//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_UTIL_READER_DOCUMENT_HPP
#define HEADER_SUPERTUX_UTIL_READER_DOCUMENT_HPP

#include <istream>
#include <sexp/value.hpp>

#include "util/reader_object.hpp"

/** The ReaderDocument holds the memory */
class ReaderDocument final
{
public:
  static ReaderDocument parse(std::istream& stream, const std::string& filename = "<stream>");
  static ReaderDocument parse(const std::string& filename);

public:
  ReaderDocument();
  ReaderDocument(const std::string& filename, sexp::Value sx);

  ReaderObject get_root() const;
  std::string get_filename() const;

private:
  std::string m_filename;
  sexp::Value m_sx;
};

#endif

/* EOF */
