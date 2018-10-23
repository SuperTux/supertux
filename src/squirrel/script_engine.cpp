//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "squirrel/script_engine.hpp"

#include "squirrel/script_interface.hpp"
#include "squirrel/scripting.hpp"
#include "squirrel/squirrel_error.hpp"
#include "squirrel/squirrel_util.hpp"
#include "supertux/game_object.hpp"
#include "util/log.hpp"

ScriptEngine::ScriptEngine() :
  m_vm(Scripting::current()->get_vm()),
  m_table(),
  m_scripts()
{
  // garbage collector has to be invoked manually
  sq_collectgarbage(m_vm);

  sq_newtable(m_vm);
  sq_pushroottable(m_vm);
  if(SQ_FAILED(sq_setdelegate(m_vm, -2)))
    throw SquirrelError(m_vm, "Couldn't set table delegate");

  sq_resetobject(&m_table);
  if (SQ_FAILED(sq_getstackobj(m_vm, -1, &m_table))) {
    throw SquirrelError(m_vm, "Couldn't get table");
  }

  sq_addref(m_vm, &m_table);
  sq_pop(m_vm, 1);
}

ScriptEngine::~ScriptEngine()
{
  for(auto& script: m_scripts)
  {
    sq_release(m_vm, &script);
  }
  m_scripts.clear();
  sq_release(m_vm, &m_table);

  sq_collectgarbage(m_vm);
}

void
ScriptEngine::expose_self(const std::string& name)
{
  sq_pushroottable(m_vm);
  store_object(m_vm, name.c_str(), m_table);
  sq_pop(m_vm, 1);
}

void
ScriptEngine::unexpose_self(const std::string& name)
{
  sq_pushroottable(m_vm);
  delete_table_entry(m_vm, name.c_str());
  sq_pop(m_vm, 1);
}

void
ScriptEngine::try_expose(GameObject& object)
{
  auto script_object = dynamic_cast<ScriptInterface*>(&object);
  if (script_object != nullptr) {
    sq_pushobject(m_vm, m_table);
    script_object->expose(m_vm, -1);
    sq_pop(m_vm, 1);
  }
}

void
ScriptEngine::try_unexpose(GameObject& object)
{
  auto script_object = dynamic_cast<ScriptInterface*>(&object);
  if (script_object != nullptr) {
    SQInteger oldtop = sq_gettop(m_vm);
    sq_pushobject(m_vm, m_table);
    try {
      script_object->unexpose(m_vm, -1);
    } catch(std::exception& e) {
      log_warning << "Couldn't unregister object: " << e.what() << std::endl;
    }
    sq_settop(m_vm, oldtop);
  }
}

void
ScriptEngine::unexpose(const std::string& name)
{
  SQInteger oldtop = sq_gettop(m_vm);
  sq_pushobject(m_vm, m_table);
  try {
    unexpose_object(m_vm, -1, name.c_str());
  } catch(std::exception& e) {
    log_warning << "Couldn't unregister object: " << e.what() << std::endl;
  }
  sq_settop(m_vm, oldtop);
}

void
ScriptEngine::run_script(const std::string& script, const std::string& sourcename)
{
  if (script.empty()) return;

  std::istringstream stream(script);
  run_script(stream, sourcename);
}

void
ScriptEngine::run_script(std::istream& in, const std::string& sourcename)
{
  try
  {
    // garbage collect thread list
    for(auto i = m_scripts.begin(); i != m_scripts.end(); ) {
      HSQOBJECT& object = *i;
      HSQUIRRELVM vm = object_to_vm(object);

      if(sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
        sq_release(m_vm, &object);
        i = m_scripts.erase(i);
        continue;
      }

      ++i;
    }

    HSQOBJECT object = create_thread(m_vm);
    m_scripts.push_back(object);

    HSQUIRRELVM vm = object_to_vm(object);

    // set root table
    sq_pushobject(vm, m_table);
    sq_setroottable(vm);

    compile_and_run(vm, in, sourcename);
  }
  catch(const std::exception& e)
  {
    log_warning << "Error running script: " << e.what() << std::endl;
  }
}

/* EOF */
