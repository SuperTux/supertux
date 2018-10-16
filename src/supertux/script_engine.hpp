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

class ScriptEngine
{
public:
  ScriptEngine();
  virtual ~ScriptEngine();

  void try_expose(GameObjectPtr object);
  void try_unexpose(GameObjectPtr object);

  /** Convenience function that takes an std::string instead of an
      std::istream& */
  HSQUIRRELVM run_script(const std::string& script, const std::string& sourcename);

  /** runs a script in the context of the sector (sector_table will be
      the roottable of this squirrel VM) */
  HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename);

protected:
  HSQOBJECT m_table;
  std::vector<HSQOBJECT> m_scripts;

private:
  ScriptEngine(const ScriptEngine&) = delete;
  ScriptEngine& operator=(const ScriptEngine&) = delete;
};

#endif

/* EOF */
