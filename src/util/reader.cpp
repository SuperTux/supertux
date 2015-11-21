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

ReaderDocument
ReaderDocument::parse(std::istream& stream)
{
  sexp::Value sx = sexp::Parser::from_stream(stream);
  return ReaderDocument(std::move(sx));
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

#if 0
    // FIXME: Woot!?
    if(translate && g_dictionary_manager) {
      std::string rel_dir = dirname(filename);
      if(rel_dir.empty())
      {
        // Relative dir inside PhysFS search path?
        // Get full path from search path, instead.
        rel_dir = PHYSFS_getRealDir(filename.c_str());
      }
      g_dictionary_manager->add_directory (rel_dir);
    }
#endif

    return parse(in);
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

ReaderIterator::ReaderIterator(const sexp::Value* sx) :
  m_root(sx),
  m_sx(nullptr)
{
  assert(m_root);
}

bool
ReaderIterator::next()
{
  if (m_root)
  {
    m_sx = m_root;
    m_root = nullptr;
    return !m_sx->is_nil();
  }
  else
  {
    assert(m_sx);

    m_sx = &m_sx->get_cdr();
    return !m_sx->is_nil();
  }
}

bool
ReaderIterator::is_string()
{
  assert(m_sx);

  return m_sx->get_car().is_string();
}

bool
ReaderIterator::is_pair()
{
  assert(m_sx);

  return m_sx->get_car().is_cons();
}

std::string
ReaderIterator::as_string()
{
  assert(m_sx);

  return m_sx->get_car().as_string();
}

std::string
ReaderIterator::get_name() const
{
  assert(m_sx);

  return m_sx->get_car().get_car().as_string();
}

bool
ReaderIterator::get(bool& value) const
{
  assert(m_sx);

  return m_sx->get_car().get_cdr().get_car().as_bool();
}

bool
ReaderIterator::get(int& value) const
{
  assert(m_sx);

  value = m_sx->get_car().get_cdr().get_car().as_int();
  return true;
}

bool
ReaderIterator::get(float& value) const
{
  assert(m_sx);

  value = m_sx->get_car().get_cdr().get_car().as_float();
  return true;
}

bool
ReaderIterator::get(std::string& value) const
{
  assert(m_sx);

  value = m_sx->get_car().get_cdr().get_car().as_string();
  return true;
}

bool
ReaderIterator::get(ReaderMapping& value) const
{
  assert(m_sx);

  value = ReaderMapping(&m_sx->get_car().get_cdr());
  return true;
}

ReaderMapping
ReaderIterator::as_mapping() const
{
  ReaderMapping result;
  get(result);
  return result;
}

ReaderMapping::ReaderMapping(const sexp::Value* sx) :
  m_sx(sx)
{
  assert(m_sx);
}

ReaderMapping::ReaderMapping() :
  m_sx(nullptr)
{
}

ReaderIterator
ReaderMapping::get_iter() const
{
  assert(m_sx);
  return ReaderIterator(m_sx);
}

bool
ReaderMapping::get(const char* key, bool& value) const
{
  assert(m_sx);

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
  assert(m_sx);

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
  assert(m_sx);

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
  assert(m_sx);

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
  assert(m_sx);

  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (sx.is_cons() &&
      sx.get_car().is_string())
  {
    value = sx.get_car().as_string();
    return true;
  }
  else if (sx.is_cons() &&
           sx.get_car().is_cons() &&
           sx.get_car().get_car().is_symbol() &&
           sx.get_car().get_car().as_string() == "_" &&
           sx.get_car().get_cdr().get_car().is_string())
  {
    value = _(sx.get_car().get_cdr().get_car().as_string());
    return true;
  }
  else
  {
    return false;
  }
}

bool
ReaderMapping::get(const char* key, std::vector<float>& value) const
{
  assert(m_sx);

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
  assert(m_sx);

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
  assert(m_sx);

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
  assert(m_sx);

  auto const& sx = sexp::assoc_ref(*m_sx, key);
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
  assert(m_sx);

  auto const& sx = sexp::assoc_ref(*m_sx, key);
  if (!sx.is_nil()) {
    value = ReaderCollection(&sx);
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
  assert(m_sx);

  std::vector<ReaderObject> result;
  for(auto const& sx : sexp::ListAdapter(*m_sx))
  {
    result.push_back(ReaderObject(&sx));
  }
  return result;
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
  assert(m_sx);

  if (m_sx->is_cons() &&
      m_sx->get_car().is_symbol())
  {
    return m_sx->get_car().as_string();
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

  if (m_sx->is_cons() &&
      (m_sx->get_cdr().is_cons() || m_sx->get_cdr().is_nil()))
  {
    return ReaderMapping(&m_sx->get_cdr());
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

  if (m_sx->is_cons() &&
      (m_sx->get_cdr().is_cons() || m_sx->get_cdr().is_nil()))
  {
    return ReaderCollection(&m_sx->get_cdr());
  }
  else
  {
    throw std::runtime_error("malformed file structure");
  }
}

/* EOF */
