//  SuperTux - Worldmap Spawnpoint
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

#include "worldmap/spawn_point.hpp"

#include <stdexcept>

#include "util/log.hpp"
#include "util/reader_mapping.hpp"

namespace worldmap {

SpawnPoint::SpawnPoint(const ReaderMapping& mapping) :
  m_name(),
  m_pos(-1.0f, -1.0f),
  m_auto_dir(Direction::NONE)
{
  mapping.get("name", m_name);
  mapping.get("x", m_pos.x);
  mapping.get("y", m_pos.y);

  std::string auto_dir_str;
  if (mapping.get("auto-dir", auto_dir_str)) {
    m_auto_dir = string_to_direction(auto_dir_str);
  }

  if (m_name.empty())
  {
    log_warning << "Spawn point at location '" << m_pos
                << "' does not have a name" << std::endl;
  }

  if (m_pos.x < 0 || m_pos.y < 0)
  {
    log_warning << "Spawn point '" << m_name << "' has invalid coordinates: "
                << m_pos << std::endl;
  }
}

} // namespace worldmap

/* EOF */
