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

#include "supertux/constants.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/world.hpp"
#include "util/string_util.hpp"

#include <string>

class EditorProject
{
private:
  /**
   * Checks if the filename belongs to an autosave file
   * @return true if the filename is an autosave file, otherwise false
   */
  static bool is_autosave_file(const std::string& filename) {
    return StringUtil::has_suffix(filename, "~");
  }

  /**
   * Returns the level's filename from an autosave file
   * @param filename filename of the level
   */
  static std::string get_levelname_from_autosave(const std::string& filename) {
    return is_autosave_file(filename) ? filename.substr(0, filename.size() - 1) : filename;
  }

  /**
   * Returns the autosave filename from a level's filename
   * @param filename filename of the level
   */
  static std::string get_autosave_from_levelname(const std::string& filename) {
    return is_autosave_file(filename) ? filename : filename + "~";
  }

public:
  EditorProject();

  /**
   * Returns the current project's world instance
   */
  inline World* get_world() const { return m_world.get(); }

  /**
   * Sets the current world for this project
   * @param world The world to set as current world
   */
  inline void set_world(std::unique_ptr<World> world) { m_world = std::move(world); }

  /**
   * Returns the level directory of the currently edited level
   */
  std::string get_level_directory() const;

  /**
   * Returns the filename of the current level
   */
  inline const std::string& get_level_file() const { return m_levelfile; }

  /**
   * Sets the filename of the current level
   */
  inline void set_level_file(const std::string& levelfile) { m_levelfile = levelfile; }

  /**
   * Returns the complete path of the current level
   */
  std::string get_level_path() const;

  /**
   * Returns the currently edited level
   */
  inline Level* get_level() const { return m_level.get(); }

  /**
   * Returns the current level as an editable instance
   */
  std::unique_ptr<Level> get_editable_level();

  /**
   * Sets the currently edited level
   * @param level The level to set as currently edited level
   * @param reset If ""true"", the current tileset is loaded from the specific level
   */
  void set_level(std::unique_ptr<Level> level, bool reset = true);

  /**
   * Reloads the current level from the latest autosave
   */
  void reload_level();

  /**
   * Returns ""true"" if a level is currently loaded, otherwise false
   */
  inline bool is_level_loaded() const { return m_level_loaded; }

  /**
   * Sets the current project to an empty level with default values
   */
  void create_empty_project();

  /**
   * Returns ""true"" when the current level is a temporary level
   * that hasn't been saved yet.
   */
  inline bool is_temp_level() const { return m_temp_level; }

  /**
   * Returns ""true"", when the current project is a worldmap
   */
  inline bool is_worldmap() const { return StringUtil::has_suffix(get_level_file(), ".stwm"); }

  /**
   * Returns the currently edited sector
   */
  inline Sector* get_sector() { return m_sector; }

  /**
   * Sets the currently edited sector
   * @param sector The sector to edit
   */
  void set_sector(Sector* sector);

  /**
   * Loads the specified sector and sets some default settings for the sector
   * @param name The name of the sector to load (default: "main")
   * @param reset If ""true"", the main sector and camera position will be reset, 
   *              Otherwise, sector and camera position are taken from the
   *              previously edited level
   */
  void load_sector(const std::string& name = DEFAULT_SECTOR_NAME, bool reset = true);

  /**
   * Returns the tileset of the current level
   * @return Returns the current level's tileset
   */
  inline TileSet* get_tileset() const { return m_tileset; }

  /**
   * Sets the tileset of the current level
   * @param tileset The tileset of the current level
   */
  inline void set_tileset(TileSet* tileset) { m_tileset = tileset; }

  /**
   * Reactivates the editor project after testing a level
   */
  void reactivate();

  /**
   * Closes the current editor project
   */
  void close();

  /**
   * Resets the current editor project to default values
   */
  void reset();

  /**
   * Open the level directory with the current system's file explorer
   */
  void open_level_directory();

  /**
   * Saves the level under the specified filename
   * @param filename The filename to save the level under
   * @param switch_file If ""true"", the current file gets switched with the specified filename
   * @param post_save callback function that gets executed once the file was saved
   */
  bool save_level(const std::string& filename = "", bool switch_file = false, const std::function<void ()>& post_save = nullptr);
  
  /**
   * Triggers the `post_save` callback function after a save
   */
  void trigger_post_save();

  /**
   * Adds the current value of `dt_sec` to the elapsed time
   * since last autosave and performs an autosave if necessary
   * @param dt_sec elapsed time since last tick
   */
  void check_autosave(float dt_sec);

  /**
   * Autosaves the current state of the currently edited level
   */
  void autosave();

  /**
   * Removes the autosave file for the current level
   */
  void remove_autosave_file();

  /**
   * Packages the current project into an installable SuperTux add-on ZIP file
   */
  void pack_addon();

  /** 
   * Checks whether the level can be saved and does not contain
   * obvious issues (currently: check if main sector and a spawn point
   * named "main" is present)
   * @param callback Function to execute in case the check succeeds
   */
  void check_save_prerequisites(const std::function<void ()>& callback) const;

  /**
   * Returns ""true"" when the current project contains unsaved changes
   */
  bool has_unsaved_changes() const;

  /**
   * Checks if the project contains unsaved changes.
   * @param action Callback function to execute once the file has been saved
   */
  void check_unsaved_changes(const std::function<void ()>& action);

  /**
   * Starts the current project in-game.
   * @param start_pos Tuple of sector name and spawn position to start the game at.
   * @return `true` if loading the project for test was successful, otherwise `false`.
   */
  bool test_project(const std::optional<std::pair<std::string, Vector>>& start_pos = std::nullopt);

  /**
   * Gets the current tileset's tilegroups.
   */
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