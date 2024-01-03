//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#include "supertux/sector_base.hpp"

namespace Base {

Sector::Sector(const std::string& type) :
  m_name(),
  m_init_script(),
  m_squirrel_environment(new SquirrelEnvironment(SquirrelVirtualMachine::current()->get_vm(), type))
{
}

void
Sector::run_script(const std::string& script, const std::string& sourcename)
{
  m_squirrel_environment->run_script(script, sourcename);
}

void
Sector::run_script(const std::string& script, const std::string& sourcename,
                   const GameObject& object, std::map<std::string, std::string> triggers)
{
  if (!object.get_name().empty())
  {
    // Delete any existing trigger references
    m_squirrel_environment->modify_table([&object](SquirrelVM& vm) {
        try
        {
          vm.get_table_entry("triggers");
          vm.delete_table_entry(object.get_name().c_str());
        }
        catch (...) {}
      });

    for (const auto& [name, ref_name] : triggers)
      m_squirrel_environment->create_reference(name, ref_name, "triggers", object.get_name());
  }

  m_squirrel_environment->run_script(script, sourcename);
}

bool
Sector::before_object_add(GameObject& object)
{
  m_squirrel_environment->try_expose(object);
  return true;
}

void
Sector::before_object_remove(GameObject& object)
{
  m_squirrel_environment->try_unexpose(object);
}

} // namespace Base

/* EOF */
