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
#include <iostream>

#include "editor/editor.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

UndoManager::UndoManager() :
  m_max_snapshots(100),
  m_index_pos(),
  m_undo_stack(),
  m_redo_stack()
{
}

void
UndoManager::try_snapshot(Level& level)
{
  std::ostringstream out;
  level.save(out);
  std::string level_snapshot = out.str();

  if (m_undo_stack.empty())
  {
    push_undo_stack(std::move(level_snapshot));
  }
  else if (level_snapshot == m_undo_stack.back())
  {
    log_debug << "skipping snapshot as nothing has changed" << std::endl;
  }
  else // level_snapshot changed
  {
    push_undo_stack(std::move(level_snapshot));
  }
}

void
UndoManager::debug_print(const char* action)
{
  if ((false))
  {
    std::cout << action << std::endl;
    std::cout << "undo_stack: ";
    for(size_t i = 0; i < m_undo_stack.size(); ++i) {
      std::cout << static_cast<const void*>(m_undo_stack[i].data()) << " ";
    }
    std::cout << std::endl;

    std::cout << "redo_stack: ";
    for(size_t i = 0; i < m_redo_stack.size(); ++i) {
      std::cout << static_cast<const void*>(m_redo_stack[i].data()) << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }
}

void
UndoManager::push_undo_stack(std::string&& level_snapshot)
{
  log_info << "doing snapshot" << std::endl;

  m_redo_stack.clear();
  m_undo_stack.push_back(std::move(level_snapshot));
  m_index_pos += 1;

  cleanup();

  debug_print("snapshot");
}

void
UndoManager::cleanup()
{
  while (m_undo_stack.size() > m_max_snapshots) {
    m_undo_stack.erase(m_undo_stack.end() - m_max_snapshots,
                       m_undo_stack.end());
  }
}

std::unique_ptr<Level>
UndoManager::undo()
{
  if (m_undo_stack.size() < 2) return {};

  m_redo_stack.push_back(std::move(m_undo_stack.back()));
  m_undo_stack.pop_back();

  std::istringstream in(m_undo_stack.back());
  ReaderMapping::s_translations_enabled = false;
  auto level = LevelParser::from_stream(in, "<undo_stack>", Editor::current()->get_level()->is_worldmap(), true);
  ReaderMapping::s_translations_enabled = true;

  m_index_pos -= 1;

  debug_print("undo");

  return level;
}

std::unique_ptr<Level>
UndoManager::redo()
{
  if (m_redo_stack.empty()) return {};

  m_undo_stack.push_back(std::move(m_redo_stack.back()));
  m_redo_stack.pop_back();

  m_index_pos += 1;

  std::istringstream in(m_undo_stack.back());
  ReaderMapping::s_translations_enabled = false;
  auto level = LevelParser::from_stream(in, "<redo_stack>", Editor::current()->get_level()->is_worldmap(), true);
  ReaderMapping::s_translations_enabled = true;

  debug_print("redo");

  return level;
}

/* EOF */
