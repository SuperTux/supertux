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

#ifndef HEADER_SUPERTUX_SQUIRREL_SQUIRREL_UTIL_HPP
#define HEADER_SUPERTUX_SQUIRREL_SQUIRREL_UTIL_HPP

#include <assert.h>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "squirrel/squirrel_virtual_machine.hpp"
#include "squirrel/squirrel_error.hpp"
#include "scripting/wrapper.hpp"

typedef std::vector<HSQOBJECT> SquirrelObjectList;

std::string squirrel2string(HSQUIRRELVM vm, SQInteger i, bool lisp_format = false);
void print_squirrel_stack(HSQUIRRELVM vm);

SQInteger squirrel_read_char(SQUserPointer file);

HSQUIRRELVM object_to_vm(const HSQOBJECT& object);

void compile_script(HSQUIRRELVM vm, std::istream& in,
                    const std::string& sourcename);
void compile_and_run(HSQUIRRELVM vm, std::istream& in,
                     const std::string& sourcename);

template<typename T>
void expose_object(HSQUIRRELVM vm, SQInteger table_idx,
                   std::unique_ptr<T> object, const std::string& name)
{
  sq_pushstring(vm, name.c_str(), -1);
  scripting::create_squirrel_instance(vm, object.release(), true);

  if (table_idx < 0)
    table_idx -= 2;

  // register instance in root table
  if (SQ_FAILED(sq_createslot(vm, table_idx))) {
    std::ostringstream msg;
    msg << "Couldn't register object '" << name << "' in squirrel table";
    throw SquirrelError(vm, msg.str());
  }
}

static inline void unexpose_object(HSQUIRRELVM vm, SQInteger table_idx,
                                   const std::string& name)
{
  assert(name.length() < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));
  sq_pushstring(vm, name.c_str(), static_cast<SQInteger>(name.length()));

  if (table_idx < 0)
    table_idx -= 1;

  if (SQ_FAILED(sq_deleteslot(vm, table_idx, SQFalse))) {
    std::ostringstream msg;
    msg << "Couldn't unregister object '" << name << "' in squirrel root table";
    throw SquirrelError(vm, msg.str());
  }
}

#endif

/* EOF */
