/*
 * src/util/reader.cpp - Utility functions for config handling.
 * Copyright (C) 2010  Florian Forster
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *   Florian "octo" Forster <supertux at octo.it>
 */

#include "util/reader.hpp"

#include <fstream>
#include <sexp/parser.hpp>
#include <sexp/util.hpp>
#include <sexp/io.hpp>

#include "physfs/ifile_stream.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "util/gettext.hpp"
#include "video/drawing_request.hpp"

int reader_get_layer(const ReaderMapping& reader, int def)
{
  int tmp = 0;
  bool status;

  status = reader.get("z-pos", tmp);

  if (!status)
    status = reader.get("layer", tmp);

  if (!status)
    tmp = def;

  if (tmp > (LAYER_GUI - 100))
    tmp = LAYER_GUI - 100;

  return (tmp);
}

namespace {

std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos) {
    return {};
  } else {
    return filename.substr(0, p);
  }
}

} // namespace

void register_translation_directory(const std::string& filename)
{
  if (g_dictionary_manager) {
    std::string rel_dir = dirname(filename);
    if (rel_dir.empty()) {
      // Relative dir inside PhysFS search path?
      // Get full path from search path, instead.
      const char* rel_dir_c = PHYSFS_getRealDir(filename.c_str());
      if (rel_dir_c) {
        rel_dir = rel_dir_c;
      }
    }

    if (!rel_dir.empty()) {
      g_dictionary_manager->add_directory(rel_dir);
    }
  }
}

ReaderDocument
ReaderDocument::parse(std::istream& stream, const std::string& filename)
{
  sexp::Value sx = sexp::Parser::from_stream(stream, sexp::Parser::USE_ARRAYS);
  return ReaderDocument(filename, std::move(sx));
}

ReaderDocument
ReaderDocument::parse(const std::string& filename)
{
  log_debug << "ReaderDocument::parse: " << filename << std::endl;

  IFileStreambuf ins(filename);
  std::istream in(&ins);

  if(!in.good()) {
    std::stringstream msg;
    msg << "Parser problem: Couldn't open file '" << filename << "'.";
    throw std::runtime_error(msg.str());
  } else {
    return parse(in, filename);
  }
}

ReaderDocument::ReaderDocument() :
  m_filename(),
  m_sx()
{
}

ReaderDocument::ReaderDocument(const std::string& filename, sexp::Value sx) :
  m_filename(filename),
  m_sx(std::move(sx))
{
}

ReaderObject
ReaderDocument::get_root() const
{
  return ReaderObject(this, &m_sx);
}

std::string
ReaderDocument::get_filename() const
{
  return m_filename;
}

ReaderIterator::ReaderIterator(const ReaderDocument* doc, const sexp::Value* sx) :
  m_doc(doc),
  m_arr(sx->as_array()),
  m_idx(0)
{
}

bool
ReaderIterator::next()
{
  m_idx += 1;
  return m_idx < m_arr.size();
}

bool
ReaderIterator::is_string()
{
  return m_arr[m_idx].is_string();
}

bool
ReaderIterator::is_pair()
{
  return m_arr[m_idx].is_array();
}

std::string
ReaderIterator::as_string_item()
{
  return m_arr[m_idx].as_string();
}

std::string
ReaderIterator::get_name() const
{
  return m_arr[m_idx].as_array()[0].as_string();
}

void
ReaderIterator::get(bool& value) const
{
  value = m_arr[m_idx].as_array()[1].as_bool();
}

void
ReaderIterator::get(int& value) const
{
  value = m_arr[m_idx].as_array()[1].as_int();
}

void
ReaderIterator::get(float& value) const
{
  value = m_arr[m_idx].as_array()[1].as_float();
}

void
ReaderIterator::get(std::string& value) const
{
  value = m_arr[m_idx].as_array()[1].as_string();
}

ReaderMapping
ReaderIterator::as_mapping() const
{
  return ReaderMapping(m_doc, &m_arr[m_idx]);
}

ReaderMapping::ReaderMapping() :
  m_doc(nullptr),
  m_sx(nullptr),
  m_arr(nullptr)
{
}

ReaderMapping::ReaderMapping(const ReaderDocument* doc, const sexp::Value* sx) :
  m_doc(doc),
  m_sx(sx),
  m_arr(&m_sx->as_array())
{
}

ReaderIterator
ReaderMapping::get_iter() const
{
  assert(m_sx);
  return ReaderIterator(m_doc, m_sx);
}

const sexp::Value*
ReaderMapping::get_item(const char* key) const
{
  for(size_t i = 1; i < m_arr->size(); ++i)
  {
    if ((*m_arr)[i].as_array()[0].as_string() == key)
    {
      return &((*m_arr)[i]);
    }
  }
  return nullptr;
}

#define GET_VALUE_MACRO(type, checker, getter)                          \
  auto const sx = get_item(key);                                        \
  if (!sx) {                                                            \
    return false;                                                       \
  } else {                                                              \
    auto const& item = sx->as_array();                                  \
    if (item.size() == 2 && item[1].checker()) {                        \
      value = item[1].getter();                                         \
      return true;                                                      \
    } else {                                                            \
      std::ostringstream msg;                                           \
      msg << m_doc->get_filename() << ":" << item[1].get_line()         \
          << ": ReaderMapping::get(key, " type "): invalid type: "      \
          << *sx;                                                       \
      throw std::runtime_error(msg.str());                              \
    }                                                                   \
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
    auto const& item = sx->as_array();
    if (item.size() == 2 && item[1].is_string()) {
      value = item[1].as_string();
      return true;
    } else if (item.size() == 2 &&
               item[1].is_array() &&
               item[1].as_array().size() == 2 &&
               item[1].as_array()[0].is_symbol() &&
               item[1].as_array()[0].as_string() == "_" &&
               item[1].as_array()[1].is_string()) {
      value = item[1].as_array()[1].as_string();
      return true;
    } else {
      std::ostringstream msg;
      msg << m_doc->get_filename() << ":" << item[1].get_line()
          << ": ReaderMapping::get(key, string): invalid type: "
          << *sx;
      throw std::runtime_error(msg.str());
    }
  }
}

#define GET_VALUES_MACRO(type, checker, getter)                         \
  auto const sx = get_item(key);                                        \
  if (!sx) {                                                            \
    return false;                                                       \
  } else {                                                              \
    auto const& item = sx->as_array();                                  \
    for(size_t i = 1; i < item.size(); ++i)                             \
    {                                                                   \
      if (item[i].checker()) {                                          \
        value.emplace_back(item[i].getter());                           \
      } else {                                                          \
        std::ostringstream msg;                                         \
        msg << m_doc->get_filename() << ":" << item[i].get_line()       \
            << ": ReaderMapping::get(key, " type "): invalid type: "    \
            << *sx;                                                     \
        throw std::runtime_error(msg.str());                            \
      }                                                                 \
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

ReaderCollection::ReaderCollection(const ReaderDocument* doc, const sexp::Value* sx) :
  m_doc(doc),
  m_sx(sx)
{
}

ReaderCollection::ReaderCollection() :
  m_doc(nullptr),
  m_sx(nullptr)
{
}

std::vector<ReaderObject>
ReaderCollection::get_objects() const
{
  assert(m_sx);

  std::vector<ReaderObject> result;
  auto const& arr = m_sx->as_array();
  for(size_t i = 1; i < arr.size(); ++i)
  {
    result.push_back(ReaderObject(m_doc, &arr[i]));
  }
  return result;
}

ReaderObject::ReaderObject(const ReaderDocument* doc, const sexp::Value* sx) :
  m_doc(doc),
  m_sx(sx)
{
}

ReaderObject::ReaderObject() :
  m_doc(nullptr),
  m_sx(nullptr)
{
}

std::string
ReaderObject::get_name() const
{
  assert(m_sx);

  if (m_sx->is_array() &&
      m_sx->as_array()[0].is_symbol())
  {
    return m_sx->as_array()[0].as_string();
  }
  else
  {
    throw std::runtime_error("malformed file structure");
  }
}

ReaderMapping
ReaderObject::get_mapping() const
{
  assert(m_sx);

  if (m_sx->is_array())
  {
    return ReaderMapping(m_doc, m_sx);
  }
  else
  {
    throw std::runtime_error("malformed file structure");
  }
}

ReaderCollection
ReaderObject::get_collection() const
{
  assert(m_sx);

  if (m_sx->is_array())
  {
    return ReaderCollection(m_doc, m_sx);
  }
  else
  {
    throw std::runtime_error("malformed file structure");
  }
}

/* EOF */
