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

#ifndef HEADER_SUPERTUX_UTIL_READER_MAPPING_HPP
#define HEADER_SUPERTUX_UTIL_READER_MAPPING_HPP

#include "util/reader_iterator.hpp"

namespace sexp {
class Value;
} // namespace sexp

class ReaderDocument;
class ReaderCollection;

class ReaderMapping final
{
public:
  ReaderMapping();

  // sx should point to (section (name value)...)
  ReaderMapping(const ReaderDocument* doc, const sexp::Value* sx);

  ReaderIterator get_iter() const;

  bool get(const char* key, bool& value) const;
  bool get(const char* key, int& value) const;
  bool get(const char* key, uint32_t& value) const;
  bool get(const char* key, float& value) const;
  bool get(const char* key, std::string& value) const;

  bool get(const char* key, std::vector<float>& value) const;
  bool get(const char* key, std::vector<std::string>& value) const;
  bool get(const char* key, std::vector<unsigned int>& value) const;

  bool get(const char* key, ReaderMapping&) const;
  bool get(const char* key, ReaderCollection&) const;

  const sexp::Value& get_sexp() const { return *m_sx; }

  const ReaderDocument* get_doc() const { return m_doc; }

private:
  /** Returns pointer to (key value) */
  const sexp::Value* get_item(const char* key) const;

private:
  const ReaderDocument* m_doc;
  const sexp::Value* m_sx;
  std::vector<sexp::Value> const* m_arr;
};

#endif

/* EOF */
