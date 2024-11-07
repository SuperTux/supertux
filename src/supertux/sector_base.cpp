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

#include "squirrel/squirrel_virtual_machine.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace Base {

Sector::Sector(const std::string& type, bool squirrel_subtable) :
  m_type_string(type),
  m_use_squirrel_subtable(squirrel_subtable),
  m_name(),
  m_init_script(),
  m_squirrel_environment()
{
}

void
Sector::finish_construction(bool editable)
{
  for (auto& object : get_objects())
    object->finish_construction();

  if (!editable)
    m_squirrel_environment = std::make_unique<SquirrelEnvironment>(SquirrelVirtualMachine::current()->get_vm(),
                                                                   m_type_string, m_use_squirrel_subtable ? m_name : "");
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
  if (m_squirrel_environment)
    m_squirrel_environment->run_script(script, sourcename);
}

void
Sector::expose()
{
  if (!m_squirrel_environment)
    return;

  m_squirrel_environment->expose_self();

  for (const auto& object : get_objects())
    m_squirrel_environment->expose(*object, object->get_name());

  // The Sector object is called 'settings' as it is accessed as 'sector.settings'
  m_squirrel_environment->expose(*this, "settings");
}

void
Sector::unexpose()
{
  if (!m_squirrel_environment)
    return;

  m_squirrel_environment->unexpose_self();

  for (const auto& object : get_objects())
    m_squirrel_environment->unexpose(object->get_name());

  m_squirrel_environment->unexpose("settings");
}

bool
Sector::before_object_add(GameObject& object)
{
  if (m_squirrel_environment)
    m_squirrel_environment->expose(object, object.get_name());

  return true;
}

void
Sector::before_object_remove(GameObject& object)
{
  if (m_squirrel_environment)
    m_squirrel_environment->unexpose(object.get_name());
}

Sector::EventHandler&
Sector::get_event_handler() const
{
  assert(m_event_handler);
  return static_cast<EventHandler&>(*m_event_handler);
}

} // namespace Base

/* EOF */
