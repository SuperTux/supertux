//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_UTIL_READER_HPP
#define HEADER_SUPERTUX_UTIL_READER_HPP

#include <memory>
#include <vector>
#include <string>

#include "lisp/lisp.hpp"

#include "util/reader_fwd.hpp"

int reader_get_layer(const ReaderMapping& reader, int def);

class ReaderObject;
class ReaderMapping;
class ReaderCollection;

/** The ReaderDocument holds the memory */
class ReaderDocument
{
public:
  static ReaderDocument parse(std::istream& stream);
  static ReaderDocument parse(const std::string& filename);

public:
  ReaderDocument();

  ReaderObject get_root() const;

private:
};

class ReaderObject final
{
public:
  ReaderObject();
  ReaderObject(const lisp::Lisp* lisp);

  std::string get_name() const;
  ReaderMapping get_mapping() const;
  ReaderCollection get_collection() const;

  explicit operator bool() const { return m_impl != nullptr; }

private:
   const lisp::Lisp* m_impl;
};

class ReaderCollection final
{
public:
  ReaderCollection();
  ReaderCollection(const lisp::Lisp* impl);

  std::vector<ReaderObject> get_objects() const;

  explicit operator bool() const { return m_impl != nullptr; }

private:
  const lisp::Lisp* m_impl;
};

/** The ReaderIterator class is for backward compatibilty with old
    fileformats only, do not use it in new code, use ReaderCollection
    and ReaderMapping instead */
class ReaderIterator
{
public:
  ReaderIterator();
  ReaderIterator(const lisp::Lisp* lisp);

  bool next();

  bool is_string();
  bool is_pair();
  std::string as_string();

  const lisp::Lisp* get_cdr() const;

  std::string get_name() const;
  std::string item() const { return get_name(); }

  bool get(bool& value) const;
  bool get(int& value) const;
  bool get(float& value) const;
  bool get(std::string& value) const;
  bool get(ReaderMapping& value) const;

  ReaderObject as_object() const;
  ReaderMapping as_mapping() const;
};

class ReaderMapping final
{
public:
  ReaderMapping();
  ReaderMapping(const lisp::Lisp* lisp);

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
  bool get(const char* key, ReaderObject&) const;

  ReaderMapping get_mapping(const char* key) const;
  ReaderCollection get_collection(const char* key) const;
  ReaderObject get_object(const char* key) const;

  /** For backward compatibilty only */
  std::vector<ReaderMapping> get_all_mappings(const char* key) const;

  explicit operator bool() const { return m_impl != nullptr; }

  const lisp::Lisp* get_lisp() const { return m_impl; }

private:
  const lisp::Lisp* m_impl;
};

#endif

/* EOF */
