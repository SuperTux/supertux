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

#include "video/drawing_request.hpp"

int reader_get_layer(const ReaderMapping& reader, int def)
{
  int tmp = 0;
  bool status;

  status = reader.get ("z-pos", tmp);

  if (!status)
    status = reader.get ("layer", tmp);

  if (!status)
    tmp = def;

  if (tmp > (LAYER_GUI - 100))
    tmp = LAYER_GUI - 100;

  return (tmp);
} /* int reader_get_layer */

ReaderDocument
ReaderDocument::parse(std::istream& stream)
{
  return {};
}

ReaderDocument
ReaderDocument::parse(const std::string& filename)
{
  std::ifstream fin(filename);
  if (!fin) {
    std::ostringstream msg;
    msg << "Parser problem: Couldn't open file '" << filename << "'.";
    throw std::runtime_error(msg.str());
  } else {
    return parse(fin);
  }
}

ReaderDocument::ReaderDocument()
{
}

ReaderObject
ReaderDocument::get_root() const
{
  return {};
}

ReaderIterator::ReaderIterator()
{
}

ReaderIterator::ReaderIterator(const lisp::Lisp* lisp)
{
}

bool
ReaderIterator::next()
{
  return false;
}

bool
ReaderIterator::is_string()
{
  return false;
}

bool
ReaderIterator::is_pair()
{
  return false;
}

std::string
ReaderIterator::as_string()
{
  return {};
}

const lisp::Lisp*
ReaderIterator::get_cdr() const
{
  return nullptr;
}

std::string
ReaderIterator::get_name() const
{
  return {};
}

bool
ReaderIterator::get(bool& value) const
{
  return false;
}

bool
ReaderIterator::get(int& value) const
{
  return false;
}

bool
ReaderIterator::get(float& value) const
{
  return false;
}

bool
ReaderIterator::get(std::string& value) const
{
  return false;
}

bool
ReaderIterator::get(ReaderMapping& value) const
{
  return false;
}

ReaderObject
ReaderIterator::as_object() const
{
  return {};
}

ReaderMapping
ReaderIterator::as_mapping() const
{
  return {};
}

ReaderMapping::ReaderMapping(const lisp::Lisp* impl) :
  m_impl(impl)
{
}

ReaderMapping::ReaderMapping() :
  m_impl(nullptr)
{
}

ReaderIterator
ReaderMapping::get_iter() const
{
  return {};
}

bool
ReaderMapping::get(const char* key, bool& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, int& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, uint32_t& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, float& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, std::string& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, std::vector<float>& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, std::vector<std::string>& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, std::vector<unsigned int>& value) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, ReaderMapping&) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, ReaderCollection&) const
{
  return false;
}

bool
ReaderMapping::get(const char* key, ReaderObject&) const
{
  return false;
}

ReaderCollection::ReaderCollection(const lisp::Lisp* impl) :
  m_impl(impl)
{
}

ReaderCollection::ReaderCollection() :
  m_impl(nullptr)
{
}

std::vector<ReaderObject>
ReaderCollection::get_objects() const
{
  if (m_impl) {
    return {}; //return m_impl->get_objects();
  } else {
    return {};
  }
}

ReaderObject::ReaderObject(const lisp::Lisp* impl) :
  m_impl(impl)
{
}

ReaderObject::ReaderObject() :
  m_impl()
{
}

std::string
ReaderObject::get_name() const
{
  if (m_impl) {
    return {}; //m_impl->get_name();
  } else {
    return {};
  }
}

ReaderMapping
ReaderObject::get_mapping() const
{
  if (m_impl) {
    return {}; //m_impl->get_mapping();
  } else {
    return {};
  }
}

ReaderCollection
ReaderObject::get_collection() const
{
  if (m_impl) {
    return {}; //m_impl->get_collection();
  } else {
    return {};
  }
}

ReaderMapping
ReaderMapping::get_mapping(const char* key) const
{
  if (!m_impl)
  {
    return {};
  }
  else
  {
    ReaderMapping result;
    get(key, result);
    return result;
  }
}

std::vector<ReaderMapping>
ReaderMapping::get_all_mappings(const char* key) const
{
  return {};
}

ReaderCollection
ReaderMapping::get_collection(const char* key) const
{
  if (!m_impl)
  {
    return {};
  }
  else
  {
    ReaderCollection result;
    get(key, result);
    return result;
  }
}

ReaderObject
ReaderMapping::get_object(const char* key) const
{
  if (!m_impl)
  {
    return {};
  }
  else
  {
    ReaderObject result;
    get(key, result);
    return result;
  }
}

/* EOF */
