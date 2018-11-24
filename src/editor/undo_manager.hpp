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

#include <list>
#include <string>
#include <memory>

class Level;

class UndoManager
{
private:
public:
  UndoManager();

  void snapshot(Level& level);
  std::unique_ptr<Level> restore();
  std::unique_ptr<Level> restore_reverse();

private:
  size_t m_max_snapshots;
  std::list<std::string> m_snapshots;
  std::list<std::string>::iterator m_current_snapshot;

private:
  UndoManager(const UndoManager&) = delete;
  UndoManager& operator=(const UndoManager&) = delete;
};

#endif

/* EOF */
