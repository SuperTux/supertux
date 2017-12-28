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

#include "supertux/game_object.hpp"
#include "supertux/script_interface.hpp"

namespace scripting {

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
  int c = in->get();
  if(in->eof())
    return 0;
  return c;
}

void try_expose(const GameObjectPtr& object, const std::string& tableName)
{
  auto object_ = dynamic_cast<ScriptInterface*>(object.get());
  if(object_ != NULL) {
    object_->expose(scripting::global_vm, tableName);
  }
}

void try_unexpose(const GameObjectPtr& object, const HSQOBJECT& table)
{
  auto object_ = dynamic_cast<ScriptInterface*>(object.get());
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    SQInteger oldtop = sq_gettop(vm);
    sq_pushobject(vm, table);
    try {
      object_->unexpose(vm, -1);
    } catch(std::exception& e) {
      log_warning << "Couldn't unregister object: " << e.what() << std::endl;
    }
    sq_settop(vm, oldtop);
  }
}

HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename,
                       ScriptList& scripts, const std::string& customRootTableName)
{
    // garbage collect thread list
    for(auto i = scripts.begin(); i != scripts.end(); ) {
      HSQOBJECT& object = *i;
      HSQUIRRELVM vm = object_to_vm(object);

      if(sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
        sq_release(global_vm, &object);
        i = scripts.erase(i);
        continue;
      }

      ++i;
    }

    HSQOBJECT object = create_thread(global_vm);
    scripts.push_back(object);

    HSQUIRRELVM vm = object_to_vm(object);

    compile_and_run(vm, in, sourcename, customRootTableName);

    return vm;
}

void compile_script(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename)
{
  if(SQ_FAILED(sq_compile(vm, squirrel_read_char, &in, sourcename.c_str(), true)))
    throw SquirrelError(vm, "Couldn't parse script");
}

void compile_and_run(HSQUIRRELVM vm, std::istream& in,
                     const std::string& sourcename, const std::string& customRootTableName)
{
  using namespace Sqrat;
  try {
    std::string script_content(std::istreambuf_iterator<char>(in), {});
    Script script;
    script.CompileString(script_content);
    if(customRootTableName.empty())
    {
      script.Run();
    }
    else
    {
      last_root_table_name = customRootTableName;
      script.RunWithCustomRootTable(customRootTableName);
    }
  }
  catch(Exception& e)
  {
    log_warning << "Error running script: " << e.Message() << std::endl;
  }
  last_root_table_name = "";
}

void release_scripts(HSQUIRRELVM vm, ScriptList& scripts, HSQOBJECT& root_table)
{
  for(auto& script: scripts)
  {
    sq_release(vm, &script);
  }
  sq_release(vm, &root_table);
  sq_collectgarbage(vm);
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

void begin_table(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  sq_newtable(vm);
}

void end_table(HSQUIRRELVM vm, const char* name)
{
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Failed to create '" + std::string(name) + "' table entry");
}

void create_empty_table(HSQUIRRELVM vm, const char* name)
{
  begin_table(vm, name);
  end_table(vm, name);
}

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

void store_object(HSQUIRRELVM vm, const char* name, const HSQOBJECT& val)
{
  sq_pushstring(vm, name, -1);
  sq_pushobject(vm, val);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Couldn't add object value to table");
}

bool has_property(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2))) return false;
  sq_pop(vm, 1);
  return true;
}

float read_float(HSQUIRRELVM vm, const char* name)
{
  get_table_entry(vm, name);

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
  get_table_entry(vm, name);

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
  get_table_entry(vm, name);

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
  get_table_entry(vm, name);

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
  if (!has_property(vm, name)) return false;
  val = read_float(vm, name);
  return true;
}

bool get_int(HSQUIRRELVM vm, const char* name, int& val) {
  if (!has_property(vm, name)) return false;
  val = read_int(vm, name);
  return true;
}

bool get_string(HSQUIRRELVM vm, const char* name, std::string& val) {
  if (!has_property(vm, name)) return false;
  val = read_string(vm, name);
  return true;
}

bool get_bool(HSQUIRRELVM vm, const char* name, bool& val) {
  if (!has_property(vm, name)) return false;
  val = read_bool(vm, name);
  return true;
}

// end: serialization functions

void get_table_entry(HSQUIRRELVM vm, const std::string& name)
{
  sq_pushstring(vm, name.c_str(), -1);
  if(SQ_FAILED(sq_get(vm, -2)))
  {
    std::ostringstream msg;
    msg << "failed to get '" << name << "' table entry";
    throw scripting::SquirrelError(vm, msg.str());
  }
  else
  {
    // successfully placed result on stack
  }
}

void get_or_create_table_entry(HSQUIRRELVM vm, const std::string& name)
{
  try
  {
    get_table_entry(vm, name);
  }
  catch(std::exception& e)
  {
    create_empty_table(vm, name.c_str());
    get_table_entry(vm, name);
  }
}

void delete_table_entry(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_deleteslot(vm, -2, false)))
  {
    // Something failed while deleting the table entry.
    // Key doesn't exist?
  }
}

std::vector<std::string> get_table_keys(HSQUIRRELVM vm)
{
  std::vector<std::string> keys;

  sq_pushnull(vm);
  while(SQ_SUCCEEDED(sq_next(vm, -2)))
  {
    //here -1 is the value and -2 is the key
    const char* result;
    if(SQ_FAILED(sq_getstring(vm, -2, &result)))
    {
      throw scripting::SquirrelError(vm, "Couldn't get string value for key");
    }
    else
    {
      keys.push_back(result);
    }

    // pops key and val before the next iteration
    sq_pop(vm, 2);
  }

  return keys;
}

}

/* EOF */
