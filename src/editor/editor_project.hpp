//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#pragma once

#include "supertux/level.hpp"
#include "supertux/world.hpp"

#include <string>

class EditorProject
{
public:
  inline void set_world(std::unique_ptr<World> w) { m_world = std::move(w); }
  inline World* get_world() const { return m_world.get(); }

  std::string get_level_directory() const;
  inline const std::string& get_level_file() const { return m_levelfile; }
  inline Level* get_level() const { return m_level.get(); }
  inline void set_level(const std::string& levelfile) { m_levelfile = levelfile; }
  void set_level(std::unique_ptr<Level> level, bool reset = true);

  inline bool is_temp_level() const { return m_temp_level; }

  inline Sector* get_sector() { return m_sector; }
  void set_sector(Sector* sector);

  void reset();

  void open_level_directory();
  void save_level();

private:
  std::unique_ptr<World> m_world;
  std::shared_ptr<Level> m_level;
  TileSet* m_tileset;
  Sector* m_sector;

  bool m_temp_level;
  std::string m_levelfile;
  std::string m_autosave_levelfile;
  bool m_save_temp_level;
  float m_time_since_last_save;
}