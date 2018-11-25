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

#ifndef HEADER_SUPERTUX_EDITOR_UNDO_MANAGER_HPP
#define HEADER_SUPERTUX_EDITOR_UNDO_MANAGER_HPP

#include <vector>
#include <string>
#include <memory>

class Level;

class UndoManager
{
private:
public:
  UndoManager();

  void try_snapshot(Level& level);

  std::unique_ptr<Level> undo();
  std::unique_ptr<Level> redo();

  bool has_unsaved_changes() const
  {
    return m_index_pos != 1;
  }

  void reset_index()
  {
    m_index_pos = 1;
  }

private:
  void push_undo_stack(std::string&& level_snapshot);
  void cleanup();
  void debug_print(const char* action);

private:
  size_t m_max_snapshots;
  int m_index_pos;
  std::vector<std::string> m_undo_stack;
  std::vector<std::string> m_redo_stack;

private:
  UndoManager(const UndoManager&) = delete;
  UndoManager& operator=(const UndoManager&) = delete;
};

#endif

/* EOF */
