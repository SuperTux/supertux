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

#include "editor/undo_manager.hpp"

#include <sstream>

#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "util/log.hpp"

UndoManager::UndoManager() :
  m_max_snapshots(100),
  m_snapshots(),
  m_current_snapshot(m_snapshots.end())
{
}

void
UndoManager::snapshot(Level& level)
{
  std::ostringstream out;
  level.save(out);

  std::string level_text = out.str();
  if (!m_snapshots.empty() && level_text == m_snapshots.back())
  {
    log_info << "skipping snapshot as nothing has changed" << std::endl;
  }
  else
  {
    m_snapshots.push_back(std::move(level_text));
    m_current_snapshot = std::prev(m_snapshots.end());

    while (m_snapshots.size() > m_max_snapshots) {
      m_snapshots.pop_front();
    }
  }
}

std::unique_ptr<Level>
UndoManager::restore()
{
  if (m_current_snapshot == m_snapshots.end()) return {};

  if (m_current_snapshot != m_snapshots.begin()) {
    m_current_snapshot = std::prev(m_current_snapshot);
  }

  std::istringstream in(*m_current_snapshot);
  return LevelParser::from_stream(in);
}

std::unique_ptr<Level>
UndoManager::restore_reverse()
{
  if (m_current_snapshot == m_snapshots.end()) return {};

  m_current_snapshot = std::next(m_current_snapshot);

  if (m_current_snapshot == m_snapshots.end()) return {};

  std::istringstream in(*m_current_snapshot);
  return LevelParser::from_stream(in);
}

/* EOF */
