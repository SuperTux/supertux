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
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace Base {

Sector::Sector(const std::string& type) :
  m_name(),
  m_init_script(),
  m_squirrel_environment(new SquirrelEnvironment(SquirrelVirtualMachine::current()->get_vm(), type))
{
}

void
Sector::parse_properties(const ReaderMapping& reader)
{
  reader.get("name", m_name);
  reader.get("init-script", m_init_script);
}

void
Sector::save_properties(Writer& writer) const
{
  writer.write("name", m_name);
  writer.write("init-script", m_init_script);
}

std::string
Sector::get_properties() const
{
  std::ostringstream stream;
  Writer writer(stream);
  save_properties(writer);

  return stream.str();
}

void
Sector::run_script(const std::string& script, const std::string& sourcename)
{
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

Sector::EventHandler&
Sector::get_event_handler() const
{
  assert(m_event_handler);
  return static_cast<EventHandler&>(*m_event_handler);
}

} // namespace Base

/* EOF */
