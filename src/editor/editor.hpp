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
#include "editor/editor_project.hpp"
#include "editor/editor_properties_panel.hpp"
#include "editor/scroller_widget.hpp"
#include "editor/tilebox.hpp"
#include "editor/toolbox_widget.hpp"
#include "editor/overlay_widget.hpp"
#include "interface/control.hpp"
#include "supertux/screen.hpp"
#include "supertux/world.hpp"
#include "util/currenton.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/script_manager.hpp"
#include "util/string_util.hpp"
#include "video/surface_ptr.hpp"

class EditorToolbarWidget;
class EditorToolbarButtonWidget;
class EditorCamera;
class EditorHistoryManager;
class EditorLayersWidget;
class TileSelection;
class EditorTileConverter;
class EditorEventHandling;

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
private:
  friend class EditorTileConverter;

public:

  /**
   * InputType defines what we're currently editing
   * TODO: Rename to InputMode
   */
  enum class InputType { NONE, TILE, OBJECT };

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

  inline EditorCamera* get_camera() const { return m_camera; }
  inline EditorHistoryManager* get_history_manager() const { return m_history_manager; }

  inline EditorOverlayWidget* get_overlay_widget() const { return m_overlay_widget; }
  inline EditorToolboxWidget* get_toolbox_widget() const { return m_toolbox_widget; }
  inline EditorToolbarWidget* get_toolbar_widget() const { return m_toolbar_widget; }
  inline EditorLayersWidget* get_layers_widget() const { return m_layers_widget; }
  inline EditorTilebox& get_tilebox() const { return m_toolbox_widget->get_tilebox(); }
  inline TileSelection* get_selected_tiles() const { return get_tilebox().get_tiles(); }

  inline EditorPropertiesPanel* get_properties_panel() const { return m_properties_panel; }
  inline std::string get_selected_object_class() const { return get_tilebox().get_object(); }

  inline InputType get_input_type() const { return m_input_type; }
  inline void set_input_type(const InputType& input_type) { m_input_type = input_type; }


  inline bool has_active_toolbox_tip() const { return get_tilebox().has_active_object_tip(); }

  inline int get_tileselect_select_mode() const { return m_toolbox_widget->get_tileselect_select_mode(); }
  inline int get_tileselect_move_mode() const { return m_toolbox_widget->get_tileselect_move_mode(); }

  void set_level(std::unique_ptr<Level> level, bool reset = true);
  inline void set_level(const std::string& levelfile)
  {
    m_project->set_level_file(levelfile);
    reload_level();
  }

  inline bool is_reloading() const { return m_is_reloading; }
  inline bool is_testing_level() const { return m_testing_level; }

  void delete_current_sector();

  inline void disable_testing() { m_testing_disabled = true; }

  void change_tileset();

  const std::vector<ObjectGroup>& get_objectgroups() const;

  void edit_path(PathGameObject* path, GameObject* new_marked_object) {
    m_overlay_widget->edit_path(path, new_marked_object);
  }

  EditorTileConverter* get_tile_converter() const { return m_tile_converter.get(); }
  EditorProject* get_project() const { return m_project.get(); }
  EditorEventHandling* get_event_handling() const { return m_event_handling.get(); }

  void set_selected_object(GameObject* object)
  {
    m_selected_object = object;
    m_properties_panel->load_object_properties(object);
  }
  const GameObject* get_selected_object() const { return m_selected_object.get(); }

  void set_test_position(const std::optional<std::pair<std::string, Vector>>& test_position)
  {
    m_test_position = test_position;
  }

  const std::optional<std::pair<std::string, Vector>>& get_test_position() const
  {
    return m_test_position;
  }

  // TODO: Move elsewhere (?) EditorInputCenter perhaps?
  bool get_draggables_visible() const { return m_draggables_visible; }
  void set_draggables_visible(bool draggables_visible)
  {
    m_draggables_visible = draggables_visible;
    if (!m_draggables_visible)
      m_draggables_visible_hint.start(6.7f);
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

  /**
   * Draws the contents of the current sector
   * @param context The current DrawingContext instance
   */
  void draw_sector(DrawingContext &context);

  /**
   * Draws a hint notifying the user that draggable objects are hidden
   * and how to enable them again.
   * @param context The current DrawingContext instance
   */
  void draw_draggables_hint(DrawingContext &context);

  /**
   * Draws the current mouse pointer (and in some circumstances the "Test here" icon)
   * on top of the currently displayed sector
   * @param context The current DrawingContext instance
   */
  void draw_mouse_pointer(DrawingContext &context);

  /**
   * Draws a selection border around the currently selected object
   * @param context The current DrawingContext instance
   */
  void draw_selection_border(DrawingContext& context);

private:
  void set_sector(Sector *sector);
  void reset_level();

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

  EditorCamera* m_camera;
  EditorOverlayWidget *m_overlay_widget;
  EditorToolboxWidget* m_toolbox_widget;
  EditorLayersWidget* m_layers_widget;
  EditorToolbarWidget* m_toolbar_widget;
  EditorPropertiesPanel* m_properties_panel;
  EditorHistoryManager* m_history_manager;

  std::unique_ptr<EditorProject> m_project;
  std::unique_ptr<EditorTileConverter> m_tile_converter;
  std::unique_ptr<EditorEventHandling> m_event_handling;

  TypedUID<GameObject> m_selected_object;

  bool m_enabled;
  InputType m_input_type;
  SurfacePtr m_bgr_surface;

  bool m_draggables_visible;
  Timer m_draggables_visible_hint;

  SpritePtr m_test_icon;

private:
  Editor(const Editor&) = delete;
  Editor& operator=(const Editor&) = delete;
};
