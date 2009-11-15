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

#ifndef HEADER_SUPERTUX_SCRIPTING_SQUIRREL_UTIL_HPP
#define HEADER_SUPERTUX_SCRIPTING_SQUIRREL_UTIL_HPP

#include <sstream>

#include "scripting/squirrel_error.hpp"
#include "scripting/wrapper.hpp"

namespace Scripting {

  extern HSQUIRRELVM global_vm;

  void init_squirrel(bool enable_debugger);
  void exit_squirrel();
  void update_debugger();

  std::string squirrel2string(HSQUIRRELVM vm, SQInteger i);
  void print_squirrel_stack(HSQUIRRELVM vm);

  HSQOBJECT create_thread(HSQUIRRELVM vm);
  SQObject vm_to_object(HSQUIRRELVM vm);
  HSQUIRRELVM object_to_vm(HSQOBJECT object);

  void compile_script(HSQUIRRELVM vm, std::istream& in,
                      const std::string& sourcename);
  void compile_and_run(HSQUIRRELVM vm, std::istream& in,
                       const std::string& sourcename);

  template<typename T>
  void expose_object(HSQUIRRELVM v, SQInteger table_idx, T* object,
                     const std::string& name, bool free = false)
  {
    sq_pushstring(v, name.c_str(), -1);
    Scripting::create_squirrel_instance(v, object, free);

    if(table_idx < 0)
      table_idx -= 2;

    // register instance in root table
    if(SQ_FAILED(sq_createslot(v, table_idx))) {
      std::ostringstream msg;
      msg << "Couldn't register object '" << name << "' in squirrel table";
      throw Scripting::SquirrelError(v, msg.str());
    }
  }

  static inline void unexpose_object(HSQUIRRELVM v, SQInteger table_idx,
                                     const std::string& name)
  {
    sq_pushstring(v, name.c_str(), name.length());

    if(table_idx < 0)
      table_idx -= 1;

    if(SQ_FAILED(sq_deleteslot(v, table_idx, SQFalse))) {
      std::ostringstream msg;
      msg << "Couldn't unregister object '" << name << "' in squirrel root table";
      throw Scripting::SquirrelError(v, msg.str());
    }
  }

  // begin serialization functions
  void store_float(HSQUIRRELVM vm, const char* name, float val);
  void store_int(HSQUIRRELVM vm, const char* name, int val);
  void store_string(HSQUIRRELVM vm, const char* name, const std::string& val);
  void store_bool(HSQUIRRELVM vm, const char* name, bool val);

  bool has_float(HSQUIRRELVM vm, const char* name);
  bool has_int(HSQUIRRELVM vm, const char* name);
  bool has_string(HSQUIRRELVM vm, const char* name);
  bool has_bool(HSQUIRRELVM vm, const char* name);

  bool get_float(HSQUIRRELVM vm, const char* name, float& val);
  bool get_int(HSQUIRRELVM vm, const char* name, int& val);
  bool get_string(HSQUIRRELVM vm, const char* name, std::string& val);
  bool get_bool(HSQUIRRELVM vm, const char* name, bool& val);

  float read_float(HSQUIRRELVM vm, const char* name);
  int read_int(HSQUIRRELVM vm, const char* name);
  std::string read_string(HSQUIRRELVM vm, const char* name);
  bool read_bool(HSQUIRRELVM vm, const char* name);
  // end serialization functions

}

#endif

/* EOF */
