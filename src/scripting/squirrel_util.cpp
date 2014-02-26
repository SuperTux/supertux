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

#include "scripting/squirrel_util.hpp"

#include <config.h>

#include <stdio.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <stdarg.h>

#include "physfs/ifile_stream.hpp"
#include "supertux/console.hpp"
#include "util/log.hpp"

#ifdef ENABLE_SQDBG

static HSQREMOTEDBG debugger = NULL;
#endif

namespace scripting {

HSQUIRRELVM global_vm = NULL;

static void printfunc(HSQUIRRELVM, const char* str, ...)
{
  char buf[4096];
  va_list arglist;
  va_start(arglist, str);
  vsprintf(buf, str, arglist);
  Console::output << (const char*) buf << std::flush;
  va_end(arglist);
}

void init_squirrel(bool enable_debugger)
{
  global_vm = sq_open(64);
  if(global_vm == NULL)
    throw std::runtime_error("Couldn't initialize squirrel vm");

  if(enable_debugger) {
#ifdef ENABLE_SQDBG
    sq_enabledebuginfo(global_vm, SQTrue);
    debugger = sq_rdbg_init(global_vm, 1234, SQFalse);
    if(debugger == NULL)
      throw SquirrelError(global_vm, "Couldn't initialize squirrel debugger");

    sq_enabledebuginfo(global_vm, SQTrue);
    log_info << "Waiting for debug client..." << std::endl;
    if(SQ_FAILED(sq_rdbg_waitforconnections(debugger)))
      throw SquirrelError(global_vm, "Waiting for debug clients failed");
    log_info << "debug client connected." << std::endl;
#endif
  }

  sq_pushroottable(global_vm);
  if(SQ_FAILED(sqstd_register_bloblib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register blob lib");
  if(SQ_FAILED(sqstd_register_mathlib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register math lib");
  if(SQ_FAILED(sqstd_register_stringlib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register string lib");

  // remove rand and srand calls from sqstdmath, we'll provide our own
  sq_pushstring(global_vm, "srand", -1);
  sq_deleteslot(global_vm, -2, SQFalse);
  sq_pushstring(global_vm, "rand", -1);
  sq_deleteslot(global_vm, -2, SQFalse);

  // register supertux API
  register_supertux_wrapper(global_vm);

  sq_pop(global_vm, 1);

  // register print function
  sq_setprintfunc(global_vm, printfunc, printfunc);
  // register default error handlers
  sqstd_seterrorhandlers(global_vm);

  // try to load default script
  try {
    std::string filename = "scripts/default.nut";
    IFileStream stream(filename);
    scripting::compile_and_run(global_vm, stream, filename);
  } catch(std::exception& e) {
    log_warning << "Couldn't load default.nut: " << e.what() << std::endl;
  }
}

void exit_squirrel()
{
#ifdef ENABLE_SQDBG
  if(debugger != NULL) {
    sq_rdbg_shutdown(debugger);
    debugger = NULL;
  }
#endif

  if (global_vm)
    sq_close(global_vm);

  global_vm = NULL;
}

void update_debugger()
{
#ifdef ENABLE_SQDBG
  if(debugger != NULL)
    sq_rdbg_update(debugger);
#endif
}

std::string squirrel2string(HSQUIRRELVM v, SQInteger i)
{
  std::ostringstream os;
  switch(sq_gettype(v, i))
  {
    case OT_NULL:
      os << "<null>";
      break;
    case OT_BOOL: {
      SQBool p;
      sq_getbool(v, i, &p);
      if (p)
        os << "true";
      else
        os << "false";
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
      while(SQ_SUCCEEDED(sq_next(v,i-1)))
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
      while(SQ_SUCCEEDED(sq_next(v,i-1)))
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
  int count = sq_gettop(v);
  for(int i = 1; i <= count; ++i) {
    printf("%d: ",i);
    switch(sq_gettype(v, i))
    {
      case OT_NULL:
        printf("null");
        break;
      case OT_INTEGER: {
        SQInteger val;
        sq_getinteger(v, i, &val);
        printf("integer (%d)", static_cast<int> (val));
        break;
      }
      case OT_FLOAT: {
        SQFloat val;
        sq_getfloat(v, i, &val);
        printf("float (%f)", val);
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
  char c = in->get();
  if(in->eof())
    return 0;
  return c;
}

void compile_script(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename)
{
  if(SQ_FAILED(sq_compile(vm, squirrel_read_char, &in, sourcename.c_str(), true)))
    throw SquirrelError(vm, "Couldn't parse script");
}

void compile_and_run(HSQUIRRELVM vm, std::istream& in,
                     const std::string& sourcename)
{
  compile_script(vm, in, sourcename);

  SQInteger oldtop = sq_gettop(vm);

  try {
    sq_pushroottable(vm);
    if(SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue)))
      throw SquirrelError(vm, "Couldn't start script");
  } catch(...) {
    sq_settop(vm, oldtop);
    throw;
  }

  // we can remove the closure in case the script was not suspended
  if(sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
    sq_settop(vm, oldtop-1);
  }
}

HSQOBJECT create_thread(HSQUIRRELVM vm)
{
  HSQUIRRELVM new_vm = sq_newthread(vm, 64);
  if(new_vm == NULL)
    throw SquirrelError(vm, "Couldn't create new VM");

  HSQOBJECT vm_object;
  sq_resetobject(&vm_object);
  if(SQ_FAILED(sq_getstackobj(vm, -1, &vm_object)))
    throw SquirrelError(vm, "Couldn't get squirrel thread from stack");
  sq_addref(vm, &vm_object);

  sq_pop(vm, 1);

  return vm_object;
}

HSQOBJECT vm_to_object(HSQUIRRELVM vm)
{
  HSQOBJECT object;
  sq_resetobject(&object);
  object._unVal.pThread = vm;
  object._type = OT_THREAD;

  return object;
}

HSQUIRRELVM object_to_vm(HSQOBJECT object)
{
  if(object._type != OT_THREAD)
    return NULL;

  return object._unVal.pThread;
}

// begin: serialization functions

void store_float(HSQUIRRELVM vm, const char* name, float val)
{
  sq_pushstring(vm, name, -1);
  sq_pushfloat(vm, val);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Couldn't add float value to table");
}

void store_int(HSQUIRRELVM vm, const char* name, int val)
{
  sq_pushstring(vm, name, -1);
  sq_pushinteger(vm, val);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Couldn't add int value to table");
}

void store_string(HSQUIRRELVM vm, const char* name, const std::string& val)
{
  sq_pushstring(vm, name, -1);
  sq_pushstring(vm, val.c_str(), val.length());
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Couldn't add float value to table");
}

void store_bool(HSQUIRRELVM vm, const char* name, bool val)
{
  sq_pushstring(vm, name, -1);
  sq_pushbool(vm, val ? SQTrue : SQFalse);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Couldn't add float value to table");
}

bool has_float(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2))) return false;
  sq_pop(vm, 1);
  return true;
}

bool has_int(HSQUIRRELVM vm, const char* name)
{
  return has_float(vm, name);
}

bool has_string(HSQUIRRELVM vm, const char* name)
{
  return has_float(vm, name);
}

bool has_bool(HSQUIRRELVM vm, const char* name)
{
  return has_float(vm, name);
}

float read_float(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get float value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }

  float result;
  if(SQ_FAILED(sq_getfloat(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get float value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return result;
}

int read_int(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get int value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }

  SQInteger result;
  if(SQ_FAILED(sq_getinteger(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get int value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return result;
}

std::string read_string(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get string value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }

  const char* result;
  if(SQ_FAILED(sq_getstring(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get string value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return std::string(result);
}

bool read_bool(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get bool value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }

  SQBool result;
  if(SQ_FAILED(sq_getbool(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get bool value for '" << name << "' from table";
    throw scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return result == SQTrue;
}

bool get_float(HSQUIRRELVM vm, const char* name, float& val) {
  if (!has_float(vm, name)) return false;
  val = read_float(vm, name);
  return true;
}

bool get_int(HSQUIRRELVM vm, const char* name, int& val) {
  if (!has_int(vm, name)) return false;
  val = read_int(vm, name);
  return true;
}

bool get_string(HSQUIRRELVM vm, const char* name, std::string& val) {
  if (!has_string(vm, name)) return false;
  val = read_string(vm, name);
  return true;
}

bool get_bool(HSQUIRRELVM vm, const char* name, bool& val) {
  if (!has_bool(vm, name)) return false;
  val = read_bool(vm, name);
  return true;
}

// end: serialization functions

}

/* EOF */
