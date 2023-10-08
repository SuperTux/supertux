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

#include "squirrel/squirrel_util.hpp"

#include <config.h>

#include <stdio.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <stdarg.h>

#include "squirrel/script_interface.hpp"
#include "supertux/game_object.hpp"
#include "util/log.hpp"

std::string squirrel2string(HSQUIRRELVM v, SQInteger i)
{
  std::ostringstream os;
  switch (sq_gettype(v, i))
  {
    case OT_NULL:
      os << "<null>";
      break;
    case OT_BOOL: {
      SQBool p;
      if (SQ_SUCCEEDED(sq_getbool(v, i, &p))) {
        if (p)
          os << "true";
        else
          os << "false";
      }
      break;
    }
    case OT_INTEGER: {
      SQInteger val;
      sq_getinteger(v, i, &val);
      os << val;
      break;
    }
    case OT_FLOAT: {
      SQFloat val;
      sq_getfloat(v, i, &val);
      os << val;
      break;
    }
    case OT_STRING: {
      const SQChar* val;
      sq_getstring(v, i, &val);
      os << "\"" << val << "\"";
      break;
    }
    case OT_TABLE: {
      bool first = true;
      os << "{";
      sq_pushnull(v);  //null iterator
      while (SQ_SUCCEEDED(sq_next(v,i-1)))
      {
        if (!first) {
          os << ", ";
        }
        first = false;

        //here -1 is the value and -2 is the key
        os << squirrel2string(v, -2) << " => "
           << squirrel2string(v, -1);

        sq_pop(v,2); //pops key and val before the nex iteration
      }
      sq_pop(v, 1);
      os << "}";
      break;
    }
    case OT_ARRAY: {
      bool first = true;
      os << "[";
      sq_pushnull(v);  //null iterator
      while (SQ_SUCCEEDED(sq_next(v,i-1)))
      {
        if (!first) {
          os << ", ";
        }
        first = false;

        //here -1 is the value and -2 is the key
        // we ignore the key, since that is just the index in an array
        os << squirrel2string(v, -1);

        sq_pop(v,2); //pops key and val before the nex iteration
      }
      sq_pop(v, 1);
      os << "]";
      break;
    }
    case OT_USERDATA:
      os << "<userdata>";
      break;
    case OT_CLOSURE:
      os << "<closure>";
      break;
    case OT_NATIVECLOSURE:
      os << "<native closure>";
      break;
    case OT_GENERATOR:
      os << "<generator>";
      break;
    case OT_USERPOINTER:
      os << "userpointer";
      break;
    case OT_THREAD:
      os << "<thread>";
      break;
    case OT_CLASS:
      os << "<class>";
      break;
    case OT_INSTANCE:
      os << "<instance>";
      break;
    case OT_WEAKREF:
      os << "<weakref>";
      break;
    default:
      os << "<unknown>";
      break;
  }
  return os.str();
}

void print_squirrel_stack(HSQUIRRELVM v)
{
  printf("--------------------------------------------------------------\n");
  SQInteger count = sq_gettop(v);
  for (int i = 1; i <= count; ++i) {
    printf("%d: ",i);
    switch (sq_gettype(v, i))
    {
      case OT_NULL:
        printf("null");
        break;
      case OT_INTEGER: {
        SQInteger val;
        sq_getinteger(v, i, &val);
        printf("integer (%d)", static_cast<int>(val));
        break;
      }
      case OT_FLOAT: {
        SQFloat val;
        sq_getfloat(v, i, &val);
        printf("float (%f)", static_cast<double>(val));
        break;
      }
      case OT_STRING: {
        const SQChar* val;
        sq_getstring(v, i, &val);
        printf("string (%s)", val);
        break;
      }
      case OT_TABLE:
        printf("table");
        break;
      case OT_ARRAY:
        printf("array");
        break;
      case OT_USERDATA:
        printf("userdata");
        break;
      case OT_CLOSURE:
        printf("closure(function)");
        break;
      case OT_NATIVECLOSURE:
        printf("native closure(C function)");
        break;
      case OT_GENERATOR:
        printf("generator");
        break;
      case OT_USERPOINTER:
        printf("userpointer");
        break;
      case OT_THREAD:
        printf("thread");
        break;
      case OT_CLASS:
        printf("class");
        break;
      case OT_INSTANCE:
        printf("instance");
        break;
      case OT_WEAKREF:
        printf("weakref");
        break;
      default:
        printf("unknown?!?");
        break;
    }
    printf("\n");
  }
  printf("--------------------------------------------------------------\n");
}

SQInteger squirrel_read_char(SQUserPointer file)
{
  std::istream* in = reinterpret_cast<std::istream*> (file);
  int c = in->get();
  if (in->eof())
    return 0;
  return c;
}

void compile_script(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename)
{
  if (SQ_FAILED(sq_compile(vm, squirrel_read_char, &in, sourcename.c_str(), true)))
    throw SquirrelError(vm, "Couldn't parse script");
}

void compile_and_run(HSQUIRRELVM vm, std::istream& in,
                     const std::string& sourcename)
{
  compile_script(vm, in, sourcename);

  SQInteger oldtop = sq_gettop(vm);

  try {
    sq_pushroottable(vm);
    if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue)))
      throw SquirrelError(vm, "Couldn't start script");
  } catch(...) {
    sq_settop(vm, oldtop);
    throw;
  }

  // we can remove the closure in case the script was not suspended
  if (sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
    sq_settop(vm, oldtop-1);
  }
}

HSQUIRRELVM object_to_vm(const HSQOBJECT& object)
{
  if (object._type != OT_THREAD)
    return nullptr;

  return object._unVal.pThread;
}

/* EOF */
