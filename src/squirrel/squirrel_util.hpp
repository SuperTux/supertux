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

#include <squirrel.h>

typedef std::vector<HSQOBJECT> SquirrelObjectList;

std::string squirrel2string(HSQUIRRELVM vm, SQInteger i);
void print_squirrel_stack(HSQUIRRELVM vm);

SQInteger squirrel_read_char(SQUserPointer file);

HSQUIRRELVM object_to_vm(const HSQOBJECT& object);

void compile_script(HSQUIRRELVM vm, std::istream& in,
                    const std::string& sourcename);
void compile_and_run(HSQUIRRELVM vm, std::istream& in,
                     const std::string& sourcename);

#endif

/* EOF */
