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

#include <boost/ref.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <sexp/io.hpp>
#include <sstream>
#include <stdexcept>

#include "util/gettext.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_error.hpp"

bool ReaderMapping::s_translations_enabled = true;

ReaderMapping::ReaderMapping(const ReaderDocument& doc, const sexp::Value& sx) :
  m_doc(doc),
  m_sx(sx),
  m_arr([this]() -> decltype(m_arr){ assert_is_array(m_doc, m_sx); return m_sx.as_array();}())
{
}

ReaderIterator
ReaderMapping::get_iter() const
{
  assert_is_array(m_doc, m_sx);

  return ReaderIterator(m_doc, m_sx);
}

const sexp::Value*
ReaderMapping::get_item(const char* key) const
{
  for (size_t i = 1; i < m_arr.size(); ++i)
  {
    auto const& pair = m_arr[i];

    // size should be >=2 not >=1, but we have to allow smaller once
    // due to get_iter(), e.g. (particles-snow)
    assert_array_size_ge(m_doc, pair, 1);

    assert_is_symbol(m_doc, pair.as_array()[0]);

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
    if (default_value) {                                                \
      value = *default_value;                                           \
    }                                                                   \
    return false;                                                       \
  } else {                                                              \
    assert_array_size_eq(m_doc, *sx, 2);                                \
    assert_##checker(m_doc, sx->as_array()[1]);                         \
    value = sx->as_array()[1].getter();                                 \
    return true;                                                        \
  }

bool
ReaderMapping::get(const char* key, bool& value, const boost::optional<bool>& default_value) const
{
  GET_VALUE_MACRO("bool", is_boolean, as_bool)
}

bool
ReaderMapping::get(const char* key, int& value, const boost::optional<int>& default_value) const
{
  GET_VALUE_MACRO("int", is_integer, as_int)
}

bool
ReaderMapping::get(const char* key, uint32_t& value, const boost::optional<uint32_t>& default_value) const
{
  GET_VALUE_MACRO("uint32_t", is_integer, as_int)
}

bool
ReaderMapping::get(const char* key, float& value, const boost::optional<float>& default_value) const
{
  GET_VALUE_MACRO("float", is_real, as_float)
}

#undef GET_VALUE_MACRO

bool
ReaderMapping::get(const char* key, std::string& value, const boost::optional<const char*>& default_value) const
{
  auto const sx = get_item(key);
  if (!sx) {
    if (default_value) {
      value = *default_value;
    }
    return false;
  } else {
    assert_array_size_eq(m_doc, *sx, 2);

    auto const& item = sx->as_array();

    if (item[1].is_string()) {
      value = item[1].as_string();
      return true;
    } else if (item[1].is_array() &&
               item[1].as_array().size() == 2 &&
               item[1].as_array()[0].is_symbol() &&
               item[1].as_array()[0].as_string() == "_" &&
               item[1].as_array()[1].is_string()) {
      if (s_translations_enabled) {
        value = _(item[1].as_array()[1].as_string());
      } else {
        value = item[1].as_array()[1].as_string();
      }
      return true;
    } else {
      raise_exception(m_doc, item[1], "expected string");
    }
  }
}

#define GET_VALUES_MACRO(type, checker, getter)                         \
  auto const sx = get_item(key);                                        \
  if (!sx) {                                                            \
    return false;                                                       \
  } else {                                                              \
    assert_is_array(m_doc, *sx);                                        \
    auto const& item = sx->as_array();                                  \
    for (size_t i = 1; i < item.size(); ++i)                             \
    {                                                                   \
      assert_##checker(m_doc, item[i]);                                 \
      value.emplace_back(item[i].getter());                             \
    }                                                                   \
    return true;                                                        \
  }

bool
ReaderMapping::get(const char* key, std::vector<bool>& value) const
{
  value.clear();
  GET_VALUES_MACRO("bool", is_boolean, as_bool)
}

bool
ReaderMapping::get(const char* key, std::vector<int>& value) const
{
  value.clear();
  GET_VALUES_MACRO("int", is_integer, as_int)
}


bool
ReaderMapping::get(const char* key, std::vector<float>& value) const
{
  value.clear();
  GET_VALUES_MACRO("float", is_real, as_float)
}

bool
ReaderMapping::get(const char* key, std::vector<std::string>& value) const
{
  value.clear();
  GET_VALUES_MACRO("string", is_string, as_string)
}

bool
ReaderMapping::get(const char* key, std::vector<unsigned int>& value) const
{
  value.clear();
  GET_VALUES_MACRO("unsigned int", is_integer, as_int)
}

#undef GET_VALUES_MACRO

bool
ReaderMapping::get(const char* key, boost::optional<ReaderMapping>& value) const
{
  auto const sx = get_item(key);
  if (sx) {
    value = boost::in_place<ReaderMapping>(boost::ref(m_doc), boost::ref(*sx));
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, boost::optional<ReaderCollection>& value) const
{
  auto const sx = get_item(key);
  if (sx) {
    value = boost::in_place<ReaderCollection>(boost::ref(m_doc), boost::ref(*sx));
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, sexp::Value& value) const
{
  auto const sx = get_item(key);
  if (!sx) {
    return false;
  } else {
    assert_array_size_eq(m_doc, *sx, 2);
    value = sx->as_array()[1];
    return true;
  }
}

/* EOF */
