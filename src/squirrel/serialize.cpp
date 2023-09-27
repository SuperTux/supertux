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

#include "squirrel/serialize.hpp"

#include <iostream>
#include <sexp/value.hpp>
#include <sexp/util.hpp>

#include "squirrel/squirrel_error.hpp"
#include "squirrel/squirrel_util.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

void load_squirrel_table(HSQUIRRELVM vm, SQInteger table_idx, const ReaderMapping& mapping)
{
  if (table_idx < 0)
    table_idx -= 2;

  auto const& arr = mapping.get_sexp().as_array();
  for (size_t i = 1; i < arr.size(); ++i)
  {
    auto const& pair = arr[i].as_array();

    // Ignore key value pairs with invalid length
    if (pair.size() < 2)
    {
      log_debug << "Found key/value pair with invalid length. Ignoring." << std::endl;
      continue;
    }

    const std::string& key = pair[0].as_string();
    auto const& value = pair[1];

    // ignore empty / null values
    if (value.is_nil())
    {
      log_debug << "Found null value for key " << key << ". Ignoring." << std::endl;
      continue;
    }
    // push the key
    sq_pushstring(vm, key.c_str(), key.size());

    // push the value
    switch (value.get_type()) {
      case sexp::Value::Type::ARRAY:
        sq_newtable(vm);
        load_squirrel_table(vm, sq_gettop(vm), ReaderMapping(mapping.get_doc(), arr[i]));
        break;
      case sexp::Value::Type::INTEGER:
        sq_pushinteger(vm, value.as_int());
        break;
      case sexp::Value::Type::REAL:
        sq_pushfloat(vm, value.as_float());
        break;
      case sexp::Value::Type::STRING:
        sq_pushstring(vm, value.as_string().c_str(), -1);
        break;
      case sexp::Value::Type::BOOLEAN:
        sq_pushbool(vm, value.as_bool() ? SQTrue : SQFalse);
        break;
      case sexp::Value::Type::SYMBOL:
        log_fatal << "Unexpected symbol in file: " << value.as_string() << std::endl;
        sq_pushnull(vm);
        break;
      default:
        assert(false);
        break;
    }

    if (SQ_FAILED(sq_createslot(vm, table_idx)))
      throw SquirrelError(vm, "Couldn't create new index");
  }
}

void save_squirrel_table(HSQUIRRELVM vm, SQInteger table_idx, Writer& writer)
{
  // offset because of sq_pushnull
  if (table_idx < 0)
    table_idx -= 1;

  //iterator table
  sq_pushnull(vm);
  while (SQ_SUCCEEDED(sq_next(vm, table_idx))) {
    if (sq_gettype(vm, -2) != OT_STRING) {
      get_logging_instance(false) << "Table contains non-string key\n";
      continue;
    }
    const SQChar* key;
    sq_getstring(vm, -2, &key);

    switch (sq_gettype(vm, -1)) {
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
        if (SQ_SUCCEEDED(sq_getbool(vm, -1, &val)))
          writer.write(key, val == SQTrue);
        break;
      }
      case OT_STRING: {
        const SQChar* str;
        sq_getstring(vm, -1, &str);
        writer.write(key, reinterpret_cast<const char*> (str));
        break;
      }
      case OT_TABLE:
      case OT_ARRAY: {
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
        get_logging_instance(false) << "Can't serialize key '" << key
                                    << "' in table.\n";
        break;
    }
    sq_pop(vm, 2);
  }
  sq_pop(vm, 1);
}

/* EOF */
