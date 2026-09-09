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
#include "editor/editor_event_handling.hpp"
#include "editor/editor_project.hpp"
#include "editor/editor_tile_converter.hpp"
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

static const float CAMERA_MIN_ZOOM = 0.39f;
static const float CAMERA_MAX_ZOOM = 3.0f;
static const float CAMERA_ZOOM_SENSITIVITY = 0.05f;
static const float CAMERA_ZOOM_FOCUS_PROGRESSION = 8.f;
class Editor final : public Screen,
                     public Currenton<Editor>
{
public:
  using exit_cb_t = std::function<void()>;

  static bool is_active();

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

  bool get_enabled() const { return m_enabled; }
  void set_enabled(bool enabled) { m_enabled = enabled; }

  bool has_focus() const;
  void event(const SDL_Event& ev) override;
  void on_window_resize() override;

  inline EditorOverlayWidget* get_overlay_widget() const { return m_overlay_widget; }
  inline EditorToolboxWidget* get_toolbox_widget() const { return m_toolbox_widget; }
  inline EditorToolbarWidget* get_toolbar_widget() const { return m_toolbar_widget; }
  inline EditorLayersWidget* get_layers_widget() const { return m_layers_widget; }
  inline EditorTilebox& get_tilebox() const { return m_toolbox_widget->get_tilebox(); }
  inline TileSelection* get_selected_tiles() const { return get_tilebox().get_tiles(); }
  inline std::string get_selected_object_class() const { return get_tilebox().get_object(); }

  inline EditorTilebox::InputType get_tileselect_input_type() const { return get_tilebox().get_input_type(); }

  inline bool has_active_toolbox_tip() const { return get_tilebox().has_active_object_tip(); }

  inline int get_tileselect_select_mode() const { return m_toolbox_widget->get_tileselect_select_mode(); }
  inline int get_tileselect_move_mode() const { return m_toolbox_widget->get_tileselect_move_mode(); }

  void set_level(std::unique_ptr<Level> level, bool reset = true);
  inline void set_level(const std::string& levelfile)
  {
    m_project->set_level_file(levelfile);
    reload_level();
  }
  void set_sector(Sector* sector);

  /**
   * Saves the level under the specified filename
   * @param filename The filename to save the level under or empty string to save with default filename
   * @param switch_file If ""true"", the current file gets switched with the specified filename
   * @param post_save callback function that gets executed once the file was saved
   */
  void save_level(const std::string& filename = "", bool switch_file = false, const std::function<void ()>& post_save = nullptr, bool save_temp_level = false)
  {
    m_project->save_level(filename, switch_file, post_save, save_temp_level);
    m_enabled = true;
  }

  inline bool is_reloading() const { return m_is_reloading; }
  inline bool is_testing_level() const { return m_testing_level; }

  void delete_current_sector();

  inline bool get_draggables_visible() { return m_show_draggables; }

  inline void disable_testing() { m_testing_disabled = true; }

  void select_tilegroup(int id);
  void change_tileset();

  void select_objectgroup(int id);
  const std::vector<ObjectGroup>& get_objectgroups() const;

  float get_scroll_speed() const { return m_scroll_speed; }
  void set_scroll_speed(float scroll_speed) { m_scroll_speed = scroll_speed; }
  
  void scroll(const Vector& velocity);

  float get_camera_scale() const { return m_new_scale; }
  void set_camera_scale(float value) { m_new_scale = value; }
  void update_camera(Camera& camera, float dt_sec);
  void keep_camera_in_bounds();

  void edit_path(PathGameObject* path, GameObject* new_marked_object) {
    m_overlay_widget->edit_path(path, new_marked_object);
  }

  EditorTileConverter* get_tile_converter() const { return m_tile_converter.get(); }
  EditorProject* get_project() const { return m_project.get(); }
  EditorEventHandling* get_event_handling() const { return m_event_handling.get(); }

  bool get_properties_panel_visible() const;

  /**
   * Checks if `pos` is inside the properties panel's area
   * (Meh, this is temporary until I move the new properties panel code elsewhere)
   */
  bool pos_in_properties_panel(const Vector& pos) const
  {
    if (m_controls.empty())
      return false;
    
    auto area = Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f);
    return area.contains(pos);
  }

  void select_object(GameObject* object);

  void retoggle_undo_tracking();
  void undo_stack_cleanup();

  void undo();
  void redo();
  void set_undo_disabled(bool state);
  void set_redo_disabled(bool state);

  void set_test_position(const std::optional<std::pair<std::string, Vector>>& test_position)
  {
    m_test_position = test_position;
  }

  const std::optional<std::pair<std::string, Vector>>& get_test_position() const
  {
    return m_test_position;
  }

  // TODO: Move elsewhere (?) EditorInputCenter perhaps?
  bool get_show_draggables() const { return m_show_draggables; }
  void set_show_draggables(bool show_draggables)
  {
    m_show_draggables = show_draggables;
    if (!m_show_draggables)
      m_show_draggables_hint.start(6.7f);
  }

  /**
   * @param filename    If non-empty, save to this file instead.
   * @param switch_file If true, the level editor will bind itself to the new
   *                    filename; subsequest saves will by default save to the
   *                    new filename.
   */
  void test_level(const std::optional<std::pair<std::string, Vector>>& test_pos = std::nullopt);

  /**
   * Deactivates the editor so that it doesn't receive
   * any keyboard or mouse events (e.g. when a menu is shown)
   */
  void deactivate();

  /**
   * Reloads the current level (e.g. after changing it)
   */
  void reload_level();

  /**
   * Method that gets called after the editor was reactivated
   * after testing a level
   */
  void reactivate_after_level_test();
  
  /**
   * Method that gets called after reactivating after showing
   * the menu
   */
  void reactivate_after_menu_close();

  /**
   * Opens the particle editor
   */
  void open_particle_editor();

  /**
   * Exits the editor
   */
  void exit();

  inline void on_exit(exit_cb_t exit_cb) { m_on_exit_cb = exit_cb; }

private:
  void reset_level();

  void add_control(const std::string& name, std::unique_ptr<InterfaceControl> new_control, const std::string& description = "");

public:
  bool m_testing_disabled;

  ScriptManager m_script_manager;

  exit_cb_t m_on_exit_cb;

  bool m_tilebox_something_selected;

private:
  bool m_is_reloading;
  bool m_testing_level;
  bool m_after_setup; // Set to true after setup function finishes and to false after leave function finishes

  std::optional<std::pair<std::string, Vector>> m_test_position;
  std::vector<std::unique_ptr<Widget> > m_widgets;
  std::vector<std::unique_ptr<InterfaceControl>> m_controls;

  EditorOverlayWidget* m_overlay_widget;
  EditorToolboxWidget* m_toolbox_widget;
  EditorLayersWidget* m_layers_widget;
  EditorToolbarWidget* m_toolbar_widget;

  std::unique_ptr<EditorProject> m_project;
  std::unique_ptr<EditorTileConverter> m_tile_converter;
  std::unique_ptr<EditorEventHandling> m_event_handling;

  TypedUID<GameObject> m_selected_object;

  bool m_enabled;
  SurfacePtr m_bgr_surface;

  float m_scroll_speed;
  float m_new_scale;
  bool m_show_draggables;
  Timer m_show_draggables_hint;

  SpritePtr m_test_icon;

private:
  Editor(const Editor&) = delete;
  Editor& operator=(const Editor&) = delete;
};
