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

#pragma once

#include <functional>
#include <vector>
#include <string>

#include <physfs.h>

#include "editor/overlay_widget.hpp"
#include "editor/tilebox.hpp"
#include "editor/toolbar_widget.hpp"
#include "editor/toolbox_widget.hpp"
#include "editor/layers_widget.hpp"
#include "editor/scroller_widget.hpp"
#include "interface/control.hpp"
#include "supertux/screen.hpp"
#include "supertux/world.hpp"
#include "util/currenton.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/script_manager.hpp"
#include "util/string_util.hpp"
#include "video/surface_ptr.hpp"

class EditorToolbarButtonWidget;
class GameObject;
class Level;
class ObjectGroup;
class Path;
class Savegame;
class Sector;
class TileSet;
class World;

class Editor final : public Screen,
                     public Currenton<Editor>
{
public:
  using exit_cb_t = std::function<void()>;

  static bool is_active();

  static void may_deactivate();
  static void may_reactivate();

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

  void event(const SDL_Event& ev) override;
  void on_window_resize() override;

  void disable_keyboard() { m_enabled = false; }

  inline Level* get_level() const { return m_level.get(); }

  inline void set_world(std::unique_ptr<World> w) { m_world = std::move(w); }
  inline World* get_world() const { return m_world.get(); }

  inline TileSet* get_tileset() const { return m_tileset; }
  inline EditorToolboxWidget* get_toolbox_widget() const { return m_toolbox_widget; }
  inline EditorToolbarWidget* get_toolbar_widget() const { return m_toolbar_widget; }
  inline EditorTilebox& get_tilebox() const { return m_toolbox_widget->get_tilebox(); }
  inline TileSelection* get_selected_tiles() const { return get_tilebox().get_tiles(); }
  inline std::string get_selected_object_class() const { return get_tilebox().get_object(); }

  inline EditorTilebox::InputType get_tileselect_input_type() const { return get_tilebox().get_input_type(); }

  inline bool has_active_toolbox_tip() const { return get_tilebox().has_active_object_tip(); }

  inline int get_tileselect_select_mode() const { return m_toolbox_widget->get_tileselect_select_mode(); }
  inline int get_tileselect_move_mode() const { return m_toolbox_widget->get_tileselect_move_mode(); }

  inline const std::string& get_levelfile() const { return m_levelfile; }

  void level_from_nothing();

  void set_level(std::unique_ptr<Level> level, bool reset = true);
  inline void set_level(const std::string& levelfile)
  {
    m_levelfile = levelfile;
    m_reload_request = true;
  }
  bool save_level(const std::string& filename = "", bool switch_file = false, const std::function<void ()>& post_save = nullptr);

  void trigger_post_save();

  std::string get_level_directory() const;

  inline bool is_temp_level() const { return m_temp_level; }

  void open_level_directory();

  inline bool is_testing_level() const { return m_leveltested; }

  void remove_autosave_file();

  /** Convert tiles on every tilemap in the level, according to a tile conversion file. */
  void convert_tiles_by_file(const std::string& file);

  void check_deprecated_tiles(bool focus = false);
  inline bool has_deprecated_tiles() const { return m_has_deprecated_tiles; }

  inline void update_autotileset() { m_overlay_widget->update_autotileset(); }

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

  inline bool get_draggables_visible() { return m_show_draggables; }

  inline void disable_testing() { m_testing_disabled = true; }

  void select_tilegroup(int id);
  void select_last_tilegroup();
  const std::vector<Tilegroup>& get_tilegroups() const;
  void change_tileset();

  void select_objectgroup(int id);
  void select_last_objectgroup();
  const std::vector<ObjectGroup>& get_objectgroups() const;

  void scroll(const Vector& velocity);

  inline bool is_level_loaded() const { return m_levelloaded; }

  void edit_path(PathGameObject* path, GameObject* new_marked_object) {
    m_overlay_widget->edit_path(path, new_marked_object);
  }

  void add_layer(GameObject* layer) { m_layers_widget->add_layer(layer); }

  inline TileMap* get_selected_tilemap() const { return m_layers_widget->get_selected_tilemap(); }

  inline Sector* get_sector() { return m_sector; }

  inline EditorLayersWidget* get_layers_widget() const { return m_layers_widget; }

  void queue_layers_refresh();

  bool get_properties_panel_visible() const;
  void select_object(GameObject* object);

  void retoggle_undo_tracking();
  void undo_stack_cleanup();

  void undo();
  void redo();
  void set_undo_disabled(bool state);
  void set_redo_disabled(bool state);

  bool has_unsaved_changes();

  void pack_addon();
  inline void on_exit(exit_cb_t exit_cb) { m_on_exit_cb = exit_cb; }

private:
  void set_sector(Sector* sector);
  void reload_level();
  void reset_level();
  void reactivate();
  void quit_editor();
  /**
   * @param filename    If non-empty, save to this file instead.
   * @param switch_file If true, the level editor will bind itself to the new
   *                    filename; subsequest saves will by default save to the
   *                    new filename.
   */
  void test_level(const std::optional<std::pair<std::string, Vector>>& test_pos);
  void update_keyboard(const Controller& controller);
  void keep_camera_in_bounds();

  void add_control(const std::string& name, std::unique_ptr<InterfaceControl> new_control, const std::string& description = "");

protected:
  std::shared_ptr<Level> m_level;
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
  bool m_save_temp_level;
  std::string m_save_request_filename;
  bool m_save_request_switch;
  bool m_test_request;
  bool m_particle_editor_request;
  bool m_testing_disabled;
  std::optional<std::pair<std::string, Vector>> m_test_pos;

  std::string* m_particle_editor_filename;

  bool m_ctrl_pressed;
  bool m_shift_pressed;
  bool m_alt_pressed;
  bool m_key_zoomed;

  ScriptManager m_script_manager;

  exit_cb_t m_on_exit_cb;

  bool m_tilebox_something_selected;

private:
  Sector* m_sector;

  bool m_levelloaded;
  bool m_leveltested;
  bool m_after_setup; // Set to true after setup function finishes and to false after leave function finishes

  TileSet* m_tileset;
  bool m_has_deprecated_tiles;
  bool m_temp_level;

  std::optional<std::pair<std::string, Vector>> m_last_test_pos;
  std::vector<std::unique_ptr<Widget> > m_widgets;
  std::vector<std::unique_ptr<InterfaceControl>> m_controls;
  std::function<void ()> m_post_save;

  EditorOverlayWidget* m_overlay_widget;
  EditorToolboxWidget* m_toolbox_widget;
  EditorLayersWidget* m_layers_widget;
  EditorToolbarWidget* m_toolbar_widget;

  TypedUID<GameObject> m_selected_object;

  bool m_enabled;
  SurfacePtr m_bgr_surface;

  float m_time_since_last_save;

  float m_scroll_speed;
  float m_new_scale;
  bool m_show_draggables;
  Timer m_show_draggables_hint;

  Vector m_mouse_pos;

  bool m_layers_widget_needs_refresh;

  SpritePtr m_shadow;

private:
  Editor(const Editor&) = delete;
  Editor& operator=(const Editor&) = delete;
};
