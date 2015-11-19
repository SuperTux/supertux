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

#include "video/drawing_request.hpp"

int reader_get_layer (const Reader& reader, int def)
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

#include "util/reader.hpp"

#include <fstream>
#include <sexp/parser.hpp>

ReaderMapping::ReaderMapping(const lisp::Lisp* impl) :
  m_impl(impl)
{
}

ReaderMapping::ReaderMapping() :
  m_impl()
{
}

ReaderCollection::ReaderCollection(const lisp::Lisp* impl) :
  m_impl(impl)
{
}

ReaderCollection::ReaderCollection() :
  m_impl()
{
}

std::vector<ReaderObject>
ReaderCollection::get_objects() const
{
  if (m_impl)
    return {}; //return m_impl->get_objects();
  else
    return {};
}

ReaderObject
ReaderObject::parse(std::istream& stream)
{
  return {};
}

ReaderObject
ReaderObject::parse(const std::string& filename)
{
  std::ifstream fin(filename);
  if (!fin)
  {
    return ReaderObject();
  }
  else
  {
    return parse(fin);
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
  if (m_impl)
    return {}; //m_impl->get_name();
  else
    return {};
}

ReaderMapping
ReaderObject::get_mapping() const
{
  if (m_impl)
    return {}; //m_impl->get_mapping();
  else
    return {};
}

ReaderCollection
ReaderObject::get_collection() const
{
  if (m_impl)
    return {}; //m_impl->get_collection();
  else
    return {};
}

bool
ReaderMapping::get  (const char* key, bool& value) const
{
  if (m_impl)
    return m_impl->get(key, value);
  else
    return false;
}

bool
ReaderMapping::get(const char* key, int& value) const
{
  if (m_impl)
    return m_impl->get(key, value);
  else
    return false;
}

bool
ReaderMapping::get(const char* key, uint32_t& value) const
{
  if (m_impl)
    return m_impl->get(key, value);
  else
    return false;
}

bool
ReaderMapping::get (const char* key, float& value) const
{
  if (m_impl)
    return m_impl->get(key, value);
  else
    return false;
}

bool
ReaderMapping::get(const char* key, std::string& value) const
{
  if (m_impl)
    return m_impl->get(key, value);
  else
    return false;
}

bool
ReaderMapping::get(const char* key, ReaderObject& object) const
{
  if (m_impl)
    return false; //m_impl->get(key, object);
  else
    return false;
}

bool
ReaderMapping::get(const char* key, ReaderMapping& mapping) const
{
  if (m_impl)
    return false; //m_impl->get(key, mapping);
  else
    return false;
}

bool
ReaderMapping::get(const char* key, ReaderCollection& collection) const
{
  if (m_impl)
    return false; // m_impl->get(key, collection);
  else
    return false;
}


ReaderMapping
ReaderMapping::get_mapping(const char* key) const
{
  if (!m_impl)
  {
    return {};
  }
  {
    ReaderMapping result;
    get(key, result);
    return result;
  }
}

ReaderCollection
ReaderMapping::get_collection(const char* key) const
{
  if (!m_impl)
  {
    return {};
  }
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
  {
    ReaderObject result;
    get(key, result);
    return result;
  }
}

std::vector<std::string>
ReaderMapping::get_keys() const
{
  if (m_impl)
    return {}; // m_impl->get_keys();
  else
    return {};
}

/* EOF */
