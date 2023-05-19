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

#include "util/log.hpp"

namespace Base {

Sector::Sector(Level& parent, const std::string& type) :
  m_level(parent),
  m_name(),
  m_init_script(),
  m_gravity(10.0f),
  m_squirrel_environment(new SquirrelEnvironment(SquirrelVirtualMachine::current()->get_vm(), type))
{
}

void
Sector::set_gravity(float gravity)
{
  if (gravity != 10.0f)
  {
    log_warning << "Changing a Sector's gravitational constant might have unforeseen side-effects: " << gravity << std::endl;
  }

  m_gravity = gravity;
}

void
Sector::run_script(const std::string& script, const std::string& sourcename)
{
  m_squirrel_environment->run_script(script, sourcename);
}

} // namespace Base

/* EOF */
