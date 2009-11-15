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
#include <config.h>

#include "scripting/serialize.hpp"

#include <memory>
#include <assert.h>
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "scripting/squirrel_error.hpp"

namespace Scripting
{

void load_squirrel_table(HSQUIRRELVM vm, SQInteger table_idx, const lisp::Lisp* lisp)
{
  using namespace lisp;

  if(table_idx < 0)
    table_idx -= 2;

  lisp::ListIterator iter(lisp);
  while(iter.next() && iter.lisp() != NULL) {
    const std::string& token = iter.item();
    sq_pushstring(vm, token.c_str(), token.size());

    const lisp::Lisp* value = iter.value();
    switch(value->get_type()) {
      case Lisp::TYPE_CONS:
        sq_newtable(vm);
        load_squirrel_table(vm, sq_gettop(vm), iter.lisp());
        break;
      case Lisp::TYPE_INTEGER:
        sq_pushinteger(vm, value->get_int());
        break;
      case Lisp::TYPE_REAL:
        sq_pushfloat(vm, value->get_float());
        break;
      case Lisp::TYPE_STRING:
        sq_pushstring(vm, value->get_string().c_str(), -1);
        break;
      case Lisp::TYPE_BOOLEAN:
        sq_pushbool(vm, value->get_bool() ? SQTrue : SQFalse);
        break;
      case Lisp::TYPE_SYMBOL:
        std::cerr << "Unexpected symbol in lisp file...";
        sq_pushnull(vm);
        break;
      default:
        assert(false);
        break;
    }

    if(SQ_FAILED(sq_createslot(vm, table_idx)))
      throw Scripting::SquirrelError(vm, "Couldn't create new index");
  }
}

void save_squirrel_table(HSQUIRRELVM vm, SQInteger table_idx, lisp::Writer& writer)
{
  // offset because of sq_pushnull
  if(table_idx < 0)
    table_idx -= 1;

  //iterator table
  sq_pushnull(vm);
  while(SQ_SUCCEEDED(sq_next(vm, table_idx))) {
    if(sq_gettype(vm, -2) != OT_STRING) {
      std::cerr << "Table contains non-string key\n";
      continue;
    }
    const SQChar* key;
    sq_getstring(vm, -2, &key);

    switch(sq_gettype(vm, -1)) {
      case OT_INTEGER: {
        SQInteger val;
        sq_getinteger(vm, -1, &val);
        writer.write(key, static_cast<int> (val));
        break;
      }
      case OT_FLOAT: {
        SQFloat val;
        sq_getfloat(vm, -1, &val);
        writer.write(key, static_cast<float> (val));
        break;
      }
      case OT_BOOL: {
        SQBool val;
        sq_getbool(vm, -1, &val);
        writer.write(key, val == SQTrue);
        break;
      }
      case OT_STRING: {
        const SQChar* str;
        sq_getstring(vm, -1, &str);
        writer.write(key, reinterpret_cast<const char*> (str));
        break;
      }
      case OT_TABLE: {
        writer.start_list(key, true);
        save_squirrel_table(vm, -1, writer);
        writer.end_list(key);
        break;
      }
      case OT_CLOSURE:
        break; // ignore
      case OT_NATIVECLOSURE:
        break;
      default:
        std::cerr << "Can't serialize key '" << key << "' in table.\n";
        break;
    }
    sq_pop(vm, 2);
  }
  sq_pop(vm, 1);
}

}
