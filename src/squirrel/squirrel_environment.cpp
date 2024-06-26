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

#include "squirrel/squirrel_environment.hpp"

#include <algorithm>

#include "squirrel/script_interface.hpp"
#include "squirrel/squirrel_error.hpp"
#include "squirrel/squirrel_scheduler.hpp"
#include "squirrel/squirrel_util.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "supertux/game_object.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

SquirrelEnvironment::SquirrelEnvironment(SquirrelVM& vm, const std::string& name) :
  m_vm(vm),
  m_table(),
  m_name(name),
  m_scripts(),
  m_scheduler(std::make_unique<SquirrelScheduler>(m_vm))
{
  // garbage collector has to be invoked manually
  sq_collectgarbage(m_vm.get_vm());

  sq_newtable(m_vm.get_vm());
  sq_pushroottable(m_vm.get_vm());
  if (SQ_FAILED(sq_setdelegate(m_vm.get_vm(), -2)))
    throw SquirrelError(m_vm.get_vm(), "Couldn't set table delegate");

  sq_resetobject(&m_table);
  if (SQ_FAILED(sq_getstackobj(m_vm.get_vm(), -1, &m_table))) {
    throw SquirrelError(m_vm.get_vm(), "Couldn't get table");
  }

  sq_addref(m_vm.get_vm(), &m_table);
  sq_pop(m_vm.get_vm(), 1);
}

SquirrelEnvironment::~SquirrelEnvironment()
{
  for (auto& script: m_scripts)
  {
    sq_release(m_vm.get_vm(), &script);
  }
  m_scripts.clear();
  sq_release(m_vm.get_vm(), &m_table);

  sq_collectgarbage(m_vm.get_vm());
}

void
SquirrelEnvironment::expose_self()
{
  sq_pushroottable(m_vm.get_vm());
  m_vm.store_object(m_name.c_str(), m_table);
  sq_pop(m_vm.get_vm(), 1);
}

void
SquirrelEnvironment::unexpose_self()
{
  sq_pushroottable(m_vm.get_vm());
  m_vm.delete_table_entry(m_name.c_str());
  sq_pop(m_vm.get_vm(), 1);
}

void
SquirrelEnvironment::try_expose(GameObject& object)
{
  auto script_object = dynamic_cast<ScriptInterface*>(&object);
  if (script_object != nullptr) {
    sq_pushobject(m_vm.get_vm(), m_table);
    script_object->expose(m_vm.get_vm(), -1);
    sq_pop(m_vm.get_vm(), 1);
  }
}

void
SquirrelEnvironment::try_unexpose(GameObject& object)
{
  auto script_object = dynamic_cast<ScriptInterface*>(&object);
  if (script_object != nullptr) {
    SQInteger oldtop = sq_gettop(m_vm.get_vm());
    sq_pushobject(m_vm.get_vm(), m_table);
    try {
      script_object->unexpose(m_vm.get_vm(), -1);
    } catch(std::exception& e) {
      log_warning << "Couldn't unregister object: " << e.what() << std::endl;
    }
    sq_settop(m_vm.get_vm(), oldtop);
  }
}

void
SquirrelEnvironment::unexpose(const std::string& name)
{
  SQInteger oldtop = sq_gettop(m_vm.get_vm());
  sq_pushobject(m_vm.get_vm(), m_table);
  try {
    unexpose_object(m_vm.get_vm(), -1, name);
  } catch(std::exception& e) {
    log_warning << "Couldn't unregister object: " << e.what() << std::endl;
  }
  sq_settop(m_vm.get_vm(), oldtop);
}

void
SquirrelEnvironment::run_script(const std::string& script, const std::string& sourcename)
{
  if (script.empty()) return;

  std::istringstream stream(script);
  run_script(stream, sourcename);
}

void
SquirrelEnvironment::garbage_collect()
{
  m_scripts.erase(
    std::remove_if(m_scripts.begin(), m_scripts.end(),
                   [this](HSQOBJECT& object){
                     HSQUIRRELVM vm = object_to_vm(object);

                     if (sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
                       sq_release(m_vm.get_vm(), &object);
                       return true;
                     } else {
                       return false;
                     }
                   }),
    m_scripts.end());
}

void
SquirrelEnvironment::run_script(std::istream& in, const std::string& sourcename)
{
  garbage_collect();

  try
  {
    HSQOBJECT object = m_vm.create_thread();
    m_scripts.push_back(object);

    HSQUIRRELVM vm = object_to_vm(object);

    sq_setforeignptr(vm, this);

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

void
SquirrelEnvironment::wait_for_seconds(HSQUIRRELVM vm, float seconds)
{
  m_scheduler->schedule_thread(vm, g_game_time + seconds, false);
}

void
SquirrelEnvironment::skippable_wait_for_seconds(HSQUIRRELVM vm, float seconds)
{
  m_scheduler->schedule_thread(vm, g_game_time + seconds, true);
}

void
SquirrelEnvironment::update(float dt_sec)
{
  m_scheduler->update(g_game_time);
}

/* EOF */
