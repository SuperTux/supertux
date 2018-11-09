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

#include "supertux/spawn_point.hpp"

#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

SpawnPoint::SpawnPoint() :
  m_name(),
  m_pos()
{}

SpawnPoint::SpawnPoint(const std::string& name, const Vector& pos) :
  m_name(name),
  m_pos(pos)
{
}

SpawnPoint::SpawnPoint(const ReaderMapping& mapping) :
  m_name(),
  m_pos(-1, -1)
{
  mapping.get("name", m_name);
  mapping.get("x", m_pos.x);
  mapping.get("y", m_pos.y);

  if (m_name.empty())
    log_warning << "No name specified for spawnpoint. Ignoring." << std::endl;

  if (m_pos.x < 0 || m_pos.y < 0)
    log_warning << "Invalid coordinates specified for spawnpoint. Ignoring." << std::endl;
}

void
SpawnPoint::save(Writer& writer)
{
  writer.start_list("spawnpoint");

  writer.write("x", m_pos.x);
  writer.write("y", m_pos.y);
  writer.write("name", m_name, false);

  writer.end_list("spawnpoint");
}

/* EOF */
