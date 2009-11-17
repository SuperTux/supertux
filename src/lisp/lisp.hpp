//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_LISP_LISP_HPP
#define HEADER_SUPERTUX_LISP_LISP_HPP

#include <assert.h>
#include <string>
#include <vector>

namespace lisp {

class Lisp
{
public:
  ~Lisp();

  enum LispType {
    TYPE_CONS,
    TYPE_SYMBOL,
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_REAL,
    TYPE_BOOLEAN
  };

  LispType get_type() const
  { return type; }

  const Lisp* get_car() const
  { return v.cons.car; }
  const Lisp* get_cdr() const
  { return v.cons.cdr; }

  bool get(std::string& val) const
  {
    if(type != TYPE_STRING && type != TYPE_SYMBOL)
      return false;
    val = v.string;
    return true;
  }

  std::string get_symbol() const
  {
    assert(type == TYPE_SYMBOL);
    return v.string;
  }

  std::string get_string() const
  {
    assert(type == TYPE_STRING);
    return v.string;
  }

  bool get(unsigned int& val) const
  {
    if(type != TYPE_INTEGER)
      return false;
    val = v.integer;
    return true;
  }

  bool get(int& val) const
  {
    if(type != TYPE_INTEGER)
      return false;
    val = v.integer;
    return true;
  }

  int get_int() const
  {
    assert(type == TYPE_INTEGER);
    return v.integer;
  }

  bool get(float& val) const
  {
    if(type != TYPE_REAL) {
      if(type == TYPE_INTEGER) {
        val = (float) v.integer;
        return true;
      }
      return false;
    }
    val = v.real;
    return true;
  }

  float get_float() const
  {
    assert(type == TYPE_REAL);
    return v.real;
  }

  bool get(bool& val) const
  {
    if(type != TYPE_BOOLEAN)
      return false;
    val = v.boolean;
    return true;
  }

  bool get_bool() const
  {
    assert(type == TYPE_BOOLEAN);
    return v.boolean;
  }

  /** convenience functions which traverse the list until a child with a
   * specified name is found. The value part is then interpreted in a specific
   * way. The functions return true, if a child was found and could be
   * interpreted correctly, otherwise false is returned and the variable value
   * is not changed.
   * (Please note that searching the lisp structure is O(n) so these functions
   *  are not a good idea for performance critical areas)
   */
  template<class T>
  bool get(const char* name, T& val) const
  {
    const Lisp* lisp = get_lisp(name);
    if(!lisp)
      return false;

    if(lisp->get_type() != TYPE_CONS)
      return false;
    lisp = lisp->get_car();
    if(!lisp)
      return false;
    return lisp->get(val);
  }

  template<class T>
  bool get(const char* name, std::vector<T>& vec) const
  {
    vec.clear();

    const Lisp* child = get_lisp(name);
    if(!child)
      return false;

    for( ; child != 0; child = child->get_cdr()) {
      T val;
      if(!child->get_car())
        continue;
      if(child->get_car()->get(val)) {
        vec.push_back(val);
      }
    }

    return true;
  }

  const Lisp* get_lisp(const char* name) const;
  const Lisp* get_lisp(const std::string& name) const
  { return get_lisp(name.c_str()); }

  // for debugging
  void print(int indent = 0) const;

private:
  friend class Parser;
  Lisp(LispType newtype);

  LispType type;
  union
  {
    struct
    {
      const Lisp* car;
      const Lisp* cdr;
    } cons;

    char* string;
    int integer;
    bool boolean;
    float real;
  } v;
};

} // end of namespace lisp

#endif

/* EOF */
