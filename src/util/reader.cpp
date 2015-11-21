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

ReaderDocument
ReaderDocument::parse(std::istream& stream)
{
  sexp::Value sx = sexp::Parser::from_stream(stream);
  return ReaderDocument(std::move(sx));
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

ReaderDocument::ReaderDocument() :
  m_sx()
{
}

ReaderDocument::ReaderDocument(sexp::Value sx) :
  m_sx(std::move(sx))
{
}

ReaderObject
ReaderDocument::get_root() const
{
  return ReaderObject(&m_sx);
}

ReaderIterator::ReaderIterator() :
  m_sx(nullptr)
{
}

ReaderIterator::ReaderIterator(const sexp::Value* sx) :
  m_sx(sx)
{
}

bool
ReaderIterator::next()
{
  if (m_sx && m_sx->is_cons()) {
    m_sx = &m_sx->get_cdr();
    return !m_sx->is_nil();
  } else {
    return false;
  }
}

bool
ReaderIterator::is_string()
{
  return m_sx->is_string();
}

bool
ReaderIterator::is_pair()
{
  return m_sx->is_cons();
}

std::string
ReaderIterator::as_string()
{
  return m_sx->as_string();
}

std::string
ReaderIterator::get_name() const
{
  return m_sx->get_car().get_car().as_string();
}

bool
ReaderIterator::get(bool& value) const
{
  return m_sx->get_car().get_cdr().get_car().as_bool();
}

bool
ReaderIterator::get(int& value) const
{
  value = m_sx->get_car().get_cdr().get_car().as_int();
  return true;
}

bool
ReaderIterator::get(float& value) const
{
  value = m_sx->get_car().get_cdr().get_car().as_float();
  return true;
}

bool
ReaderIterator::get(std::string& value) const
{
  value = m_sx->get_car().get_cdr().get_car().as_string();
  return true;
}

bool
ReaderIterator::get(ReaderMapping& value) const
{
  value = ReaderMapping(&m_sx->get_car().get_cdr());
  return true;
}

ReaderObject
ReaderIterator::as_object() const
{
  return ReaderObject(m_sx);
}

ReaderMapping
ReaderIterator::as_mapping() const
{
  return ReaderMapping(m_sx);
}

ReaderMapping::ReaderMapping(const sexp::Value* sx) :
  m_sx(sx)
{
}

ReaderMapping::ReaderMapping() :
  m_sx(nullptr)
{
}

ReaderIterator
ReaderMapping::get_iter() const
{
  return ReaderIterator(m_sx);
}

bool
ReaderMapping::get(const char* key, bool& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (sx.is_cons() && sx.get_car().is_boolean()) {
    value = sx.get_car().as_bool();
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, int& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (sx.is_cons() && sx.get_car().is_integer()) {
    value = sx.get_car().as_int();
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, uint32_t& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (sx.is_cons() && sx.get_car().is_integer()) {
    value = sx.get_car().as_int();
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, float& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (sx.is_cons() && sx.get_car().is_real()) {
    value = sx.get_car().as_float();
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, std::string& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (sx.is_cons() && sx.get_car().is_string()) {
    value = sx.get_car().as_string();
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, std::vector<float>& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_cons()) {
    return false;
  } else {
    // FIXME: how to handle type errors?
    std::vector<float> result;
    for(auto const& it : sexp::ListAdapter(sx))
    {
      if (it.is_real())
      {
        result.push_back(it.as_float());
      }
      else
      {
        return false;
      }
    }
    value = std::move(result);
    return true;
  }
}

bool
ReaderMapping::get(const char* key, std::vector<std::string>& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_cons()) {
    return false;
  } else {
    // FIXME: how to handle type errors?
    std::vector<std::string> result;
    for(auto const& it : sexp::ListAdapter(sx))
    {
      if (it.is_string())
      {
        result.push_back(it.as_string());
      }
      else
      {
        return false;
      }
    }
    value = std::move(result);
    return true;
  }
}

bool
ReaderMapping::get(const char* key, std::vector<unsigned int>& value) const
{
  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_cons()) {
    return false;
  } else {
    // FIXME: how to handle type errors?
    std::vector<unsigned int> result;
    for(auto const& it : sexp::ListAdapter(sx))
    {
      if (it.is_integer())
      {
        result.push_back(it.as_int());
      }
      else
      {
        return false;
      }
    }
    value = std::move(result);
    return true;
  }
}

bool
ReaderMapping::get(const char* key, ReaderMapping& value) const
{
  auto sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_nil()) {
    value = ReaderMapping(&sx);
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, ReaderCollection& value) const
{
  auto sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_nil()) {
    value = ReaderCollection(&sx);
    return true;
  } else {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, ReaderObject& value) const
{
  auto sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_nil()) {
    value = ReaderObject(&sx);
    return true;
  } else {
    return false;
  }
}

ReaderCollection::ReaderCollection(const sexp::Value* sx) :
  m_sx(sx)
{
}

ReaderCollection::ReaderCollection() :
  m_sx(nullptr)
{
}

std::vector<ReaderObject>
ReaderCollection::get_objects() const
{
  if (m_sx) {
    return {}; //return m_sx->get_objects();
  } else {
    return {};
  }
}

ReaderObject::ReaderObject(const sexp::Value* sx) :
  m_sx(sx)
{
}

ReaderObject::ReaderObject() :
  m_sx(nullptr)
{
}

std::string
ReaderObject::get_name() const
{
  if (m_sx) {
    if (m_sx->is_cons() &&
        m_sx->get_car().is_symbol())
    {
      return m_sx->get_car().as_string();
    }
    else
    {
      throw std::runtime_error("malformed file structure");
    }
  } else {
    return {};
  }
}

ReaderMapping
ReaderObject::get_mapping() const
{
  if (m_sx) {
    if (m_sx->is_cons() &&
        (m_sx->get_cdr().is_cons() || m_sx->get_cdr().is_nil()))
    {
      return ReaderMapping(&m_sx->get_cdr());
    }
    else
    {
      throw std::runtime_error("malformed file structure");
    }
  } else {
    return {};
  }
}

ReaderCollection
ReaderObject::get_collection() const
{
  if (m_sx) {
    return {}; //m_sx->get_collection();
  } else {
    return {};
  }
}

ReaderMapping
ReaderMapping::get_mapping(const char* key) const
{
  if (!m_sx)
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
  if (!m_sx) {
    return {};
  } else {
    // FIXME
    return {};
  }
}

ReaderCollection
ReaderMapping::get_collection(const char* key) const
{
  if (!m_sx)
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

/* EOF */
