//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_EDITOR_HPP
#define HEADER_SUPERTUX_EDITOR_EDITOR_HPP

#include <functional>
#include <vector>
#include <string>

#include <physfs.h>

#include "editor/overlay_widget.hpp"
#include "editor/toolbox_widget.hpp"
#include "editor/layers_widget.hpp"
#include "editor/scroller_widget.hpp"
#include "supertux/screen.hpp"
#include "supertux/world.hpp"
#include "util/currenton.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"
#include "video/surface_ptr.hpp"

class GameObject;
class Level;
class ObjectGroup;
class Path;
class Savegame;
class Sector;
class TileSet;
class UndoManager;
class World;

class Editor final : public Screen,
                     public Currenton<Editor>
{
public:
  static bool is_active();

  static PHYSFS_EnumerateCallbackResult foreach_recurse(void *data,
                                                        const char *origdir,
                                                        const char *fname);

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
  static bool s_resaving_in_progress;

public:
  Editor();
  ~Editor() override;

  virtual void draw(Compositor&) override;
  virtual void update(float dt_sec, const Controller& controller) override;

  virtual void setup() override;
  virtual void leave() override;

  virtual IntegrationStatus get_status() const override;

  void event(const SDL_Event& ev);
  void resize();

  void disable_keyboard() { m_enabled = false; }

  Level* get_level() const { return m_level.get(); }

  void set_world(std::unique_ptr<World> w);
  World* get_world() const { return m_world.get(); }

  TileSet* get_tileset() const { return m_tileset; }
  TileSelection* get_tiles() const { return m_toolbox_widget->get_tiles(); }
  std::string get_tileselect_object() const { return m_toolbox_widget->get_object(); }

  EditorToolboxWidget::InputType get_tileselect_input_type() const { return m_toolbox_widget->get_input_type(); }

  int get_tileselect_select_mode() const;
  int get_tileselect_move_mode() const;

  std::string get_levelfile() const { return m_levelfile; }

  void set_level(const std::string& levelfile_) {
    m_levelfile = levelfile_;
    m_reload_request = true;
  }

  std::string get_level_directory() const;

  void open_level_directory();

  bool is_testing_level() const { return m_leveltested; }

  void remove_autosave_file();

  /** Checks whether the level can be saved and does not contain
      obvious issues (currently: check if main sector and a spawn point
      named "main" is present) */
  void check_save_prerequisites(const std::function<void ()>& callback) const;
  void check_unsaved_changes(const std::function<void ()>& action);

  void load_sector(const std::string& name);
  void delete_current_sector();

  void update_node_iterators();
  void esc_press();
  void delete_markers();
  void sort_layers();

  void select_tilegroup(int id);
  const std::vector<Tilegroup>& get_tilegroups() const;
  void change_tileset();

  void select_objectgroup(int id);
  const std::vector<ObjectGroup>& get_objectgroups() const;

  void scroll(const Vector& velocity);

  bool is_level_loaded() const { return m_levelloaded; }

  void edit_path(PathGameObject* path, GameObject* new_marked_object) {
    m_overlay_widget->edit_path(path, new_marked_object);
  }

  void add_layer(GameObject* layer) { m_layers_widget->add_layer(layer); }

  TileMap* get_selected_tilemap() const { return m_layers_widget->get_selected_tilemap(); }

  Sector* get_sector() { return m_sector; }

  void undo();
  void redo();

  void pack_addon();

private:
  void set_sector(Sector* sector);
  void set_level(std::unique_ptr<Level> level, bool reset = true);
  void reload_level();
  void quit_editor();
  /**
   * @param filename    If non-empty, save to this file instead.
   * @param switch_file If true, the level editor will bind itself to the new
   *                    filename; subsequest saves will by default save to the
   *                    new filename.
   */
  void save_level(const std::string& filename = "", bool switch_file = false);
  void test_level(const boost::optional<std::pair<std::string, Vector>>& test_pos);
  void update_keyboard(const Controller& controller);

protected:
  std::unique_ptr<Level> m_level;
  std::unique_ptr<World> m_world;

  std::string m_levelfile;
  std::string m_autosave_levelfile;

public:
  bool m_quit_request;
  bool m_newlevel_request;
  bool m_reload_request;
  bool m_reactivate_request;
  bool m_deactivate_request;
  bool m_save_request;
  std::string m_save_request_filename;
  bool m_save_request_switch;
  bool m_test_request;
  bool m_particle_editor_request;
  boost::optional<std::pair<std::string, Vector>> m_test_pos;

  std::unique_ptr<Savegame> m_savegame;
  std::string* m_particle_editor_filename;

private:
  Sector* m_sector;

  bool m_levelloaded;
  bool m_leveltested;
  bool m_after_setup; // Set to true after setup function finishes and to false after leave function finishes

  TileSet* m_tileset;

  std::vector<std::unique_ptr<Widget> > m_widgets;
  EditorOverlayWidget* m_overlay_widget;
  EditorToolboxWidget* m_toolbox_widget;
  EditorLayersWidget* m_layers_widget;

  bool m_enabled;
  SurfacePtr m_bgr_surface;

  std::unique_ptr<UndoManager> m_undo_manager;
  bool m_ignore_sector_change;
  
  bool m_level_first_loaded;
  
  float m_time_since_last_save;

  float m_scroll_speed;

private:
  Editor(const Editor&) = delete;
  Editor& operator=(const Editor&) = delete;
};

#endif

/* EOF */
