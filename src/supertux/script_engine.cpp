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

#include "supertux/script_engine.hpp"

#include "scripting/scripting.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/squirrel_util.hpp"
#include "util/log.hpp"

ScriptEngine::ScriptEngine() :
  m_table(),
  m_scripts()
{
  // garbage collector has to be invoked manually
  sq_collectgarbage(scripting::global_vm);

  sq_newtable(scripting::global_vm);
  sq_pushroottable(scripting::global_vm);
  if(SQ_FAILED(sq_setdelegate(scripting::global_vm, -2)))
    throw scripting::SquirrelError(scripting::global_vm, "Couldn't set sector_table delegate");

  sq_resetobject(&m_table);
  if (SQ_FAILED(sq_getstackobj(scripting::global_vm, -1, &m_table))) {
    throw scripting::SquirrelError(scripting::global_vm, "Couldn't get table");
  }
  sq_addref(scripting::global_vm, &m_table);
  sq_pop(scripting::global_vm, 1);
}

ScriptEngine::~ScriptEngine()
{
  scripting::release_scripts(scripting::global_vm, m_scripts, m_table);
}

void
ScriptEngine::try_expose(GameObjectPtr object)
{
  scripting::try_expose(object, m_table);
}

void
ScriptEngine::try_unexpose(GameObjectPtr object)
{
  scripting::try_unexpose(object, m_table);
}

void
ScriptEngine::unexpose(const std::string& name)
{
  HSQUIRRELVM vm = scripting::global_vm;
  SQInteger oldtop = sq_gettop(vm);
  sq_pushobject(vm, m_table);
  try {
    scripting::unexpose_object(vm, -1, name.c_str());
  } catch(std::exception& e) {
    log_warning << "Couldn't unregister object: " << e.what() << std::endl;
  }
  sq_settop(vm, oldtop);
}

HSQUIRRELVM
ScriptEngine::run_script(const std::string& script, const std::string& sourcename)
{
  if(script.empty())
  {
    return nullptr;
  }
  std::istringstream stream(script);
  return run_script(stream, sourcename);
}

HSQUIRRELVM
ScriptEngine::run_script(std::istream& in, const std::string& sourcename)
{
  try {
    return scripting::run_script(in, "Sector - " + sourcename,
                                 m_scripts, &m_table);
  }
  catch(const std::exception& e)
  {
    log_warning << "Error running sector script: " << e.what() << std::endl;
    return nullptr;
  }
}

/* EOF */
