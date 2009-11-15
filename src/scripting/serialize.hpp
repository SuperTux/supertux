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

#ifndef HEADER_SUPERTUX_SCRIPTING_SERIALIZE_HPP
#define HEADER_SUPERTUX_SCRIPTING_SERIALIZE_HPP

#include <squirrel.h>
//#include <string>

namespace lisp {
class Lisp;
class Writer;
} // namespace lisp

namespace Scripting {

void save_squirrel_table(HSQUIRRELVM vm, SQInteger table_idx, lisp::Writer& writer);
void load_squirrel_table(HSQUIRRELVM vm, SQInteger table_idx, const lisp::Lisp* lisp);

} // namespace Scripting

#endif

/* EOF */
