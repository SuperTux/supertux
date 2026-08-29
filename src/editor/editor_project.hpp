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
#include "supertux/level_parser.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/world.hpp"
#include "util/string_util.hpp"

#include <string>

class EditorProject
{
private:
  static bool is_autosave_file(const std::string& filename) {
    return StringUtil::has_suffix(filename, "~");
  }
  static std::string get_levelname_from_autosave(const std::string& filename) {
    return is_autosave_file(filename) ? filename.substr(0, filename.size() - 1) : filename;
  }
  static std::string get_autosave_from_levelname(const std::string& filename) {
    return is_autosave_file(filename) ? filename : filename + "~";
  }

public:
  EditorProject();
  inline World* get_world() const { return m_world.get(); }
  inline void set_world(std::unique_ptr<World> w) { m_world = std::move(w); }

  std::string get_level_directory() const;
  inline const std::string& get_level_file() const { return m_levelfile; }
  std::string get_level_path() const;

  inline Level* get_level() const { return m_level.get(); }
  std::unique_ptr<Level> get_editable_level();
  inline void set_level(const std::string& levelfile) { m_levelfile = levelfile; }
  void set_level(std::unique_ptr<Level> level, bool reset = true);
  void reload_level();
  inline bool is_level_loaded() const { return m_level_loaded; }

  void level_from_nothing();

  inline bool is_temp_level() const { return m_temp_level; }
  inline bool is_worldmap() const { return StringUtil::has_suffix(get_level_file(), ".stwm"); }

  inline Sector* get_sector() { return m_sector; }
  void set_sector(Sector* sector);
  void load_sector(const std::string& name);

  inline TileSet* get_tileset() const { return m_tileset; }
  inline void set_tileset(TileSet* tileset) { m_tileset = tileset; }

  void reactivate();
  void close();
  void reset();

  void open_level_directory();
  bool save_level(const std::string& filename = "", bool switch_file = false, const std::function<void ()>& post_save = nullptr);
  void trigger_post_save();

  void autosave(float dt_sec);
  void remove_autosave_file();

  void pack_addon();

  /** Checks whether the level can be saved and does not contain
      obvious issues (currently: check if main sector and a spawn point
      named "main" is present) */
  void check_save_prerequisites(const std::function<void ()>& callback) const;

  bool has_unsaved_changes() const;
  void check_unsaved_changes(const std::function<void ()>& action);

  bool test_project(const std::optional<std::pair<std::string, Vector>>& start_pos = std::nullopt);

  const std::vector<Tilegroup>& get_tilegroups() const
  {
    return m_tileset->get_tilegroups();
  }

private:
  std::unique_ptr<World> m_world;
  std::shared_ptr<Level> m_level;
  TileSet* m_tileset;
  Sector* m_sector;

  bool m_temp_level;
  std::string m_levelfile;
  std::string m_autosave_levelfile;
  bool m_level_loaded;
  bool m_save_temp_level;
  float m_time_since_last_save;

  std::function<void ()> m_post_save;
};