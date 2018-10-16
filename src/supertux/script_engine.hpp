//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SCRIPT_ENGINE_HPP
#define HEADER_SUPERTUX_SUPERTUX_SCRIPT_ENGINE_HPP

#include <string>
#include <vector>

#include <squirrel.h>

#include "supertux/game_object_ptr.hpp"
#include "scripting/squirrel_util.hpp"

class ScriptInterface;

class ScriptEngine
{
public:
  ScriptEngine();
  virtual ~ScriptEngine();

protected:
  /** Expose this engine in the global_vm under 'name' */
  void expose_self(const std::string& name);
  void unexpose_self(const std::string& name);

  /** Expose the GameObject if it has a ScriptInterface, otherwise do
      nothing. */
  void try_expose(GameObjectPtr object);
  void try_unexpose(GameObjectPtr object);

  /** Generic expose function, T must be a type that has a
      create_squirrel_instance() associated with it. */
  template<typename T>
  void expose(const std::string& name, std::unique_ptr<T> script_object)
  {
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushobject(vm, m_table);
    scripting::expose_object(vm, -1, script_object.release(), name.c_str(), true);
    sq_pop(vm, 1);
  }
  void unexpose(const std::string& name);

public:
  /** Convenience function that takes an std::string instead of an
      std::istream& */
  HSQUIRRELVM run_script(const std::string& script, const std::string& sourcename);

  /** Runs a script in the context of the ScriptEngine (m_table will
      be the roottable of this squirrel VM) and keeps a reference to
      the script so the script gets destroyed when the ScriptEngine is
      destroyed). */
  HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename);

private:
  HSQOBJECT m_table;
  std::vector<HSQOBJECT> m_scripts;

private:
  ScriptEngine(const ScriptEngine&) = delete;
  ScriptEngine& operator=(const ScriptEngine&) = delete;
};

#endif

/* EOF */
