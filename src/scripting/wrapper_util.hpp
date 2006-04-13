//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __WRAPPERUTIL_HPP__
#define __WRAPPERUTIL_HPP__

#include <squirrel.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include "wrapper.hpp"
#include "squirrel_error.hpp"

namespace Scripting
{

std::string squirrel2string(HSQUIRRELVM vm, int i);
void print_squirrel_stack(HSQUIRRELVM vm);
void compile_script(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename);
void compile_and_run(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename);

template<typename T>
void expose_object(HSQUIRRELVM v, int table_idx, T* object,
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

static inline void unexpose_object(HSQUIRRELVM v, int table_idx, const std::string& name)
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

}

#endif
