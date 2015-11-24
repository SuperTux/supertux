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

#include "util/reader_mapping.hpp"

#include <sexp/io.hpp>
#include <sstream>
#include <stdexcept>

#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_error.hpp"

ReaderMapping::ReaderMapping() :
  m_doc(nullptr),
  m_sx(nullptr),
  m_arr(nullptr)
{
}

ReaderMapping::ReaderMapping(const ReaderDocument* doc, const sexp::Value* sx) :
  m_doc(doc),
  m_sx(sx),
  m_arr()
{
  assert(m_doc);
  assert(m_sx);

  assert_is_array(*m_doc, *m_sx);

  m_arr = &m_sx->as_array();
}

ReaderIterator
ReaderMapping::get_iter() const
{
  assert(m_doc);
  assert(m_sx);

  assert_is_array(*m_doc, *m_sx);

  return ReaderIterator(m_doc, m_sx);
}

const sexp::Value*
ReaderMapping::get_item(const char* key) const
{
  for(size_t i = 1; i < m_arr->size(); ++i)
  {
    auto const& pair = (*m_arr)[i];

    assert_array_size_ge(*m_doc, pair, 2);
    assert_is_symbol(*m_doc, pair.as_array()[0]);

    if (pair.as_array()[0].as_string() == key)
    {
      return &pair;
    }
  }
  return nullptr;
}

#define GET_VALUE_MACRO(type, checker, getter)                          \
  auto const sx = get_item(key);                                        \
  if (!sx) {                                                            \
    return false;                                                       \
  } else {                                                              \
    assert_array_size_eq(*m_doc, *sx, 2);                               \
    assert_##checker(*m_doc, sx->as_array()[1]);                        \
    value = sx->as_array()[1].getter();                                 \
    return true;                                                        \
  }

bool
ReaderMapping::get(const char* key, bool& value) const
{
  GET_VALUE_MACRO("bool", is_boolean, as_bool);
}

bool
ReaderMapping::get(const char* key, int& value) const
{
  GET_VALUE_MACRO("int", is_integer, as_int);
}

bool
ReaderMapping::get(const char* key, uint32_t& value) const
{
  GET_VALUE_MACRO("uint32_t", is_integer, as_int);
}

bool
ReaderMapping::get(const char* key, float& value) const
{
  GET_VALUE_MACRO("float", is_real, as_float);
}

#undef GET_VALUE_MACRO

bool
ReaderMapping::get(const char* key, std::string& value) const
{
  auto const sx = get_item(key);
  if (!sx) {
    return false;
  } else {
    assert_array_size_eq(*m_doc, *sx, 2);

    auto const& item = sx->as_array();

    if (item[1].is_string()) {
      value = item[1].as_string();
      return true;
    } else if (item[1].is_array() &&
               item[1].as_array().size() == 2 &&
               item[1].as_array()[0].is_symbol() &&
               item[1].as_array()[0].as_string() == "_" &&
               item[1].as_array()[1].is_string()) {
      value = item[1].as_array()[1].as_string();
      return true;
    } else {
      raise_exception(*m_doc, item[1], "expected string");
    }
  }
}

#define GET_VALUES_MACRO(type, checker, getter)                         \
  auto const sx = get_item(key);                                        \
  if (!sx) {                                                            \
    return false;                                                       \
  } else {                                                              \
    assert_is_array(*m_doc, *sx);                                       \
    auto const& item = sx->as_array();                                  \
    for(size_t i = 1; i < item.size(); ++i)                             \
    {                                                                   \
      assert_##checker(*m_doc, item[i]);                                \
      value.emplace_back(item[i].getter());                             \
    }                                                                   \
    return true;                                                        \
  }

bool
ReaderMapping::get(const char* key, std::vector<float>& value) const
{
  GET_VALUES_MACRO("float", is_real, as_float);
}

bool
ReaderMapping::get(const char* key, std::vector<std::string>& value) const
{
  GET_VALUES_MACRO("string", is_string, as_string);
}

bool
ReaderMapping::get(const char* key, std::vector<unsigned int>& value) const
{
  GET_VALUES_MACRO("unsigned int", is_integer, as_int);
}

#undef GET_VALUES_MACRO

bool
ReaderMapping::get(const char* key, ReaderMapping& value) const
{
  auto const sx = get_item(key);
  if (sx) {
    value = ReaderMapping(m_doc, sx);
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, ReaderCollection& value) const
{
  auto const sx = get_item(key);
  if (sx) {
    value = ReaderCollection(m_doc, sx);
    return true;
  } else {
    return false;
  }
}

/* EOF */
