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

#include "editor/editor.hpp"
#include "gui/notification.hpp"
#include "math/rectf.hpp"

#include <fstream>
#include <functional>
#include <sstream>
#include <limits>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <fmt/format.h>

#include "zip_manager.hpp"

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "editor/button_widget.hpp"
#include "editor/layer_icon.hpp"
#include "editor/object_info.hpp"
#include "editor/particle_editor.hpp"
#include "editor/resize_marker.hpp"
#include "editor/tile_selection.hpp"
#include "editor/tip.hpp"
#include "editor/tool_icon.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_script.hpp"
#include "gui/mousecursor.hpp"
#include "math/util.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/spawnpoint.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/util.hpp"
#include "sdk/integration.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/console.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"
#include "supertux/sector.hpp"
#include "supertux/sector_parser.hpp"

static const float CAMERA_MIN_ZOOM = 0.39f;
static const float CAMERA_MAX_ZOOM = 3.0f;
static const float CAMERA_ZOOM_SENSITIVITY = 0.05f;
static const float CAMERA_ZOOM_FOCUS_PROGRESSION = 8.f;

bool Editor::s_resaving_in_progress = false;

using InputType = EditorTilebox::InputType;

bool
Editor::is_active()
{
  if (s_resaving_in_progress) {
    return true;
  } else {
    auto* self = Editor::current();
    return self && !self->m_testing_level && self->m_after_setup;
  }
}


Editor::Editor() :
  m_particle_editor_request(false),
  m_particle_editor_filename(),
  m_ctrl_pressed(false),
  m_shift_pressed(false),
  m_alt_pressed(false),
  m_key_zoomed(false),
  m_pen_down(false),
  m_testing_level(false),
  m_after_setup(false),
  m_widgets(),
  m_controls(),
  m_overlay_widget(),
  m_toolbox_widget(),
  m_layers_widget(),
  m_toolbar_widget(),
  m_project(new EditorProject),
  m_tile_converter(new EditorTileConverter),
  m_selected_object(),
  m_testing_disabled(false),
  m_enabled(false),
  m_bgr_surface(Surface::from_file("images/engine/menu/bg_editor.png")),
  m_scroll_speed(32.0f),
  m_new_scale(0.f),
  m_show_draggables(true),
  m_show_draggables_hint(),
  m_mouse_pos(0.f, 0.f),
  m_layers_widget_needs_refresh(false),
  m_script_manager(),
  m_on_exit_cb(nullptr),
  m_last_test_pos(std::nullopt),
  m_test_icon(SpriteManager::current()->create("images/engine/editor/spawnpoint.png"))
{
  auto toolbox_widget = std::make_unique<EditorToolboxWidget>(*this);
  auto layers_widget = std::make_unique<EditorLayersWidget>(*this);
  auto overlay_widget = std::make_unique<EditorOverlayWidget>(*this);
  auto toolbar_widget = std::make_unique<EditorToolbarWidget>(*this);

  m_toolbox_widget = toolbox_widget.get();
  m_layers_widget = layers_widget.get();
  m_overlay_widget = overlay_widget.get();
  m_toolbar_widget = toolbar_widget.get();

  m_widgets.push_back(std::move(toolbox_widget));
  m_widgets.push_back(std::move(layers_widget));
  m_widgets.push_back(std::move(overlay_widget));
  m_widgets.push_back(std::move(toolbar_widget));
}

Editor::~Editor()
{
  if (m_on_exit_cb)
    m_on_exit_cb();

  m_script_manager.clear_tmp();
}

void
Editor::queue_layers_refresh()
{
  m_layers_widget_needs_refresh = true;
}

void
Editor::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  if (m_project->is_level_loaded())
  {
    for(const auto& widget : m_widgets)
    {
      widget->draw(context);
    }

    if (get_properties_panel_visible())
    {
      context.color().set_blur(g_config->editor_blur);
      context.color().draw_filled_rect(Rectf(0.0f, 0.0f, SCREEN_WIDTH, 32.0f),
                       Color(0.2f, 0.2f, 0.2f, 0.5f), LAYER_GUI - 6);

      context.color().draw_filled_rect(Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f),
                       Color(0.2f, 0.2f, 0.2f, 0.5f), LAYER_GUI - 6);
      context.color().set_blur(0);

      for(const auto& control : m_controls)
      {
        control->draw(context);
      }
    }

    // Avoid drawing the sector if we're about to test it, as there is a dangling pointer
    // issue with the PlayerStatus.
    if (!m_testing_level)
    {
      auto sector = m_project->get_sector();
      context.push_transform();
      context.set_max_layer(LAYER_GUI - 22); // Lowest layer used by an editor UI item is LAYER_GUI - 21

      sector->draw(context);

      context.pop_transform();

      // If an object is selected, draw an indicator around it.
      const GameObject* selected_object = m_selected_object.get();
      if (selected_object)
      {
        const MovingObject* moving_selected_obj = dynamic_cast<const MovingObject*>(selected_object);
        if (moving_selected_obj)
        {
          context.push_transform();
          const Camera& camera = sector->get_camera();
          context.set_translation(camera.get_translation());
          context.scale(camera.get_current_scale());

          const Rectf& bbox = moving_selected_obj->get_bbox();
          context.color().draw_rect(bbox.grown(10.f), Color::WHITE, LAYER_GUI + 1);
          
          // context.color().draw_line(Vector(bbox.get_right() + 10.f, bbox.get_top() - 10.f),
          //                           Vector(bbox.get_right() + 10.f, bbox.get_top()),
          //                           Color::WHITE, LAYER_GUI + 1);
          // context.color().draw_line(Vector(bbox.get_right() + 10.f, bbox.get_top() - 10.f),
          //                           Vector(bbox.get_right(), bbox.get_top() - 10.f),
          //                           Color::WHITE, LAYER_GUI + 1);
          // context.color().draw_line(Vector(bbox.get_left() - 10.f, bbox.get_bottom() + 10.f),
          //                           Vector(bbox.get_left() - 10.f, bbox.get_bottom()),
          //                           Color::WHITE, LAYER_GUI + 1);
          // context.color().draw_line(Vector(bbox.get_left() - 10.f, bbox.get_bottom() + 10.f),
          //                           Vector(bbox.get_left(), bbox.get_bottom() + 10.f),
          //                           Color::WHITE, LAYER_GUI + 1);

          context.pop_transform();
        }
      }
      else
      {
        m_selected_object = 0;
        m_controls.clear();
      }
    }

    // BEGIN Draw shadows and line
    constexpr float LINE_THICKNESS = 1.f;
    Rectf border_rect = Rectf{SCREEN_WIDTH - 128.f - LINE_THICKNESS, 0,
                              SCREEN_WIDTH - 128.f, static_cast<float>(SCREEN_HEIGHT - 32.f)};
    Color line_color = (g_config->editorcolor - Color(0.2, 0.2, 0.2, 0.2)).validate();
    context.color().draw_filled_rect(border_rect, line_color, LAYER_GUI + 1);

    Rectf shadow_rect = border_rect;
    shadow_rect.set_left(border_rect.get_left() - 16 + LINE_THICKNESS);
    shadow_rect.set_right(border_rect.get_right() - LINE_THICKNESS);
    context.color().draw_gradient(Color(0.0f, 0.0f, 0.0f, 0.0f),
                                  Color(0.0f, 0.0f, 0.0f, 0.2f),
                                  LAYER_GUI + 1,
                                  GradientDirection::HORIZONTAL,
                                  shadow_rect);

    Rectf layers_rect = Rectf{0, SCREEN_HEIGHT - 32.f - LINE_THICKNESS,
                              SCREEN_WIDTH - 128.f, SCREEN_HEIGHT - 32.f};
    context.color().draw_filled_rect(layers_rect, line_color, LAYER_GUI + 1);
    // END Draw shadows and line

    context.color().draw_filled_rect(context.get_rect(),
                                     Color(0.0f, 0.0f, 0.0f),
                                     0.0f, std::numeric_limits<int>::min());


    // Show a little indicator for testing
    if (m_ctrl_pressed && m_shift_pressed)
    {
      if (m_enabled)
        MouseCursor::current()->set_visible(false);
      context.color().draw_text(
        Resources::normal_font,
        "T",
        { m_mouse_pos.x + 12.f, m_mouse_pos.y - 16.f - 12.f }, ALIGN_LEFT, LAYER_OBJECTS+1,
        Color(1.0f, 1.0f, 0.6f, 0.8f));
      m_test_icon->draw_scaled(context.color(),
                               {{m_mouse_pos.x - 16.f, m_mouse_pos.y - 16.f}, Sizef{32.f, 32.f}},
                               LAYER_GUI + 1);
    }
    else if (m_enabled)
      MouseCursor::current()->set_visible(true);

    if (!m_show_draggables && m_show_draggables_hint.get_progress() < 1.0f)
    {
      context.color().draw_text(
        Resources::normal_font,
        _("Note: Draggables are now hidden. Press Ctrl+H to show again."),
        { 16.0f, SCREEN_HEIGHT - 64.f }, ALIGN_LEFT, LAYER_OBJECTS+1,
        Color(1.0f, 1.0f, 0.6f, (1.0f - m_show_draggables_hint.get_progress())));
    }
  }
  else
  {
    context.color().draw_surface_scaled(m_bgr_surface,
                                        context.get_rect(),
                                        -100);
  }

  if (!(m_ctrl_pressed && m_shift_pressed))
    MouseCursor::current()->set_visible(true);
}

void
Editor::update(float dt_sec, const Controller& controller)
{
  m_project->check_autosave(dt_sec);

  m_script_manager.poll();

  // TODO: TEMPORARY addition to reinstate the return after deactivating
  if (!m_enabled)
  {
    return;
  }

  if (m_particle_editor_request) {
    m_particle_editor_request = false;
    std::unique_ptr<Screen> screen(new ParticleEditor());
    if (m_particle_editor_filename)
      static_cast<ParticleEditor*>(screen.get())->open("particles/" + *m_particle_editor_filename);
    ScreenManager::current()->push_screen(std::move(screen));
    return;
  }

  auto sector = m_project->get_sector();
  if (sector == nullptr)
  {
    return;
  }

  // Update other components.
  if (m_project->is_level_loaded() && !m_testing_level) {
    BIND_SECTOR(*sector);

    for (auto& object : sector->get_objects()) {
      object->editor_update();
    }

    if (m_layers_widget_needs_refresh)
    {
      if (m_layers_widget)
      {
        m_layers_widget->refresh();
      }
      m_layers_widget_needs_refresh = false;
    }

    for (const auto& widget : m_widgets) {
      widget->update(dt_sec);
    }

    m_toolbar_widget->update(dt_sec);

    for(const auto& control : m_controls)
    {
      control->update(dt_sec);
    }

    // Now that all widgets have been updated, which should have relinquished
    // pointers to objects marked for deletion, we can actually delete them.
    for (auto& sector : m_project->get_level()->get_sectors())
      sector->flush_game_objects();

    update_keyboard(controller);
  }

  update_camera(sector->get_camera(), dt_sec);
}

void
Editor::update_camera(Camera& camera, float dt_sec)
{
  // Ensure camera is free, which is like normal but immune to the camera boundary.
  camera.set_mode(Camera::Mode::FREE);
  // If camera scale must be changed, change it here.
  if (m_new_scale != 0.f)
  {
    // Do not clamp, as to prevent pointless calls to EditorOverlayWidget::update_pos().
    if (m_new_scale >= CAMERA_MIN_ZOOM && m_new_scale <= CAMERA_MAX_ZOOM)
    {
      const bool zooming_in = camera.get_current_scale() < m_new_scale;

      camera.set_scale(m_new_scale);

      // When zooming in, focus on the position of the mouse.
      if (zooming_in && !m_key_zoomed && !g_config->editor_zoom_centered)
        camera.move((m_mouse_pos - Vector(static_cast<float>(SCREEN_WIDTH - 128),
                                          static_cast<float>(SCREEN_HEIGHT - 32)) / 2.f) / CAMERA_ZOOM_FOCUS_PROGRESSION);

      keep_camera_in_bounds();
    }
    m_key_zoomed = false;
    m_new_scale = 0.f;
  }

  camera.update(dt_sec);
}

void
Editor::test_level(const std::optional<std::pair<std::string, Vector>>& test_pos)
{
  if (m_testing_disabled)
  {
    Dialog::show_message(_("You cannot test a level when playing from the worldmap.\n\n"
                           "Exit the level editor instead."));
    return;
  }

  m_project->check_save_prerequisites([this, test_pos]()
  {
    m_testing_level = true;
    m_last_test_pos = test_pos;

    MouseCursor::current()->set_icon(nullptr);
    Tile::draw_editor_images = false;
    Compositor::s_render_lighting = true;

    if (!g_config->max_viewport && g_config->editor_max_viewport)
      VideoSystem::current()->get_viewport().force_full_viewport(false);

    bool test_successful = m_project->test_project(test_pos);
    
    if (!test_successful)
    {
      reactivate_after_level_test();
    }
  });
}

void
Editor::scroll(const Vector& velocity)
{
  if (!m_project->is_level_loaded())
    return;

  auto sector = m_project->get_sector();
  auto& camera = sector->get_camera();

  camera.move(velocity / camera.get_current_scale());
  keep_camera_in_bounds();
}

void
Editor::keep_camera_in_bounds()
{
  auto sector = m_project->get_sector();
  auto& camera = sector->get_camera();

  constexpr float offset = 80.f;
#if 0
  float controls_offset_x = m_controls.size() != 0 ? -200.f : 0.f;
  float controls_offset_y = m_controls.size() != 0 ? -32.f : 0.f;
  camera.keep_in_bounds(Rectf(-offset + controls_offset_x, -offset + controls_offset_y,
                              std::max(0.0f, m_sector->get_editor_width() + 128.f / camera.get_current_scale()) + offset,
                              std::max(0.0f, m_sector->get_editor_height() + 32.f / camera.get_current_scale()) + offset));
#endif

  camera.keep_in_bounds(Rectf(-offset,
                              -offset,
                              std::max(0.f, sector->get_editor_width()) + offset + 128.f,
                              std::max(0.f, sector->get_editor_height()) + offset));
  m_overlay_widget->update_pos();
}


void
Editor::esc_press()
{
  m_enabled = false;
  m_overlay_widget->delete_markers();
  MenuManager::instance().set_menu(MenuStorage::EDITOR_MENU);
}

void
Editor::update_keyboard(const Controller& controller)
{
  if(!has_focus())
    return;

  const bool* keys = nullptr;
  keys = SDL_GetKeyboardState(nullptr);
  assert(keys != nullptr);

  if (controller.pressed(Control::ESCAPE)) {
    esc_press();
    return;
  }

  if (controller.pressed(Control::DEBUG_MENU) && g_config->developer_mode)
  {
    m_enabled = false;
    m_overlay_widget->delete_markers();
    MenuManager::instance().set_menu(MenuStorage::DEBUG_MENU);
    return;
  }

  if (controller.hold(Control::LEFT) || keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    scroll({ -m_scroll_speed, 0.0f });
  }

  if (controller.hold(Control::RIGHT) || keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    scroll({ m_scroll_speed, 0.0f });
  }

  if (controller.hold(Control::UP) || keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    scroll({ 0.0f, -m_scroll_speed });
  }

  if (controller.hold(Control::DOWN) || keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    scroll({ 0.0f, m_scroll_speed });
  }
}

void
Editor::set_sector(Sector* sector)
{
  if (!sector) return;

  m_project->set_sector(sector);

  m_layers_widget->refresh();
  select_object(nullptr);
}

void
Editor::delete_current_sector()
{
  auto level = m_project->get_level();
  auto& sectors = level->m_sectors;

  if (sectors.size() <= 1) {
    log_fatal << "Deleting the last sector is not allowed." << std::endl;
  }

  for (auto i = sectors.begin(); i != sectors.end(); ++i) {
    if ( i->get() == m_project->get_sector() ) {
      sectors.erase(i);
      break;
    }
  }

  set_sector(sectors.front().get());
}

void
Editor::set_level(std::unique_ptr<Level> level, bool reset)
{
  m_script_manager.clear_tmp();

  m_project->set_level(std::move(level));
  m_project->load_sector(DEFAULT_SECTOR_NAME, reset);

  m_is_reloading = false;
  m_enabled = true;

  if (reset) {
    m_toolbox_widget->get_tilebox().set_input_type(InputType::NONE);
  }

  if (reset) {
    m_toolbox_widget->get_tilebox().set_input_type(InputType::TILE);
    m_toolbox_widget->get_tilebox().select_tilegroup(0);
  }

  m_layers_widget->refresh_sector_text();
  m_toolbox_widget->update_mouse_icon();
  m_overlay_widget->on_level_change();

  if (!reset) return;

  // Warn the user if any deprecated tiles are used throughout the level
  m_tile_converter->check_deprecated_tiles();
  if (m_tile_converter->has_deprecated_tiles())
  {
    std::string message = _("This level contains deprecated tiles.\nIt is strongly recommended to replace all deprecated tiles\nto avoid loss of compatibility in future versions.");
    if (!g_config->editor_show_deprecated_tiles)
      message += "\n\n" + _("Tip: Turn on \"Show Deprecated Tiles\" from the level editor menu.");

    Dialog::show_message(message);
  }
}

void
Editor::reload_level()
{
  m_is_reloading = true;

  auto level = m_project->get_editable_level();
  set_level(std::move(level));

  try
  {
    m_project->reload_level();
  }
  catch(const std::exception& e)
  {
    reset_level();
  }

  retoggle_undo_tracking();
  undo_stack_cleanup();
}

void
Editor::reset_level()
{
  m_project->reset();

  m_is_reloading = false;

  MouseCursor::current()->set_icon(nullptr);
  set_level(nullptr, true);
}

void
Editor::exit()
{
  m_project->check_unsaved_changes([this] {
    m_project->close();
    m_enabled = false;
    Tile::draw_editor_images = false;
    ScreenManager::current()->pop_screen();
#ifdef __EMSCRIPTEN__
    int persistent = EM_ASM_INT({
      return supertux2_ispersistent();
    }, 0); // EM_ASM_INT is a variadic macro and Clang requires at least 1 value for the variadic argument.
    if (!persistent)
      Dialog::show_message(_("Don't forget that your levels and assets\naren't saved between sessions!\nIf you want to keep your levels, download them\nfrom the \"Manage Assets\" menu."));
#endif
  });
}

void
Editor::leave()
{
  MouseCursor::current()->set_icon(nullptr);
  Compositor::s_render_lighting = true;
  m_after_setup = false;
}

void
Editor::setup()
{
  Tile::draw_editor_images = true;
  Sector::s_draw_solids_only = false;
  m_after_setup = true;

  if (!m_project->is_level_loaded())
  {
#if 0
    if (AddonManager::current()->is_old_addon_enabled())
    {
      auto dialog = std::make_unique<Dialog>();
      dialog->set_text(_("Some obsolete add-ons are still active\nand might cause collisions with the default SuperTux structure.\nYou can still enable these add-ons in the menu.\nDisabling these add-ons will not delete your game progress."));
      dialog->clear_buttons();

      dialog->add_default_button(_("Disable add-ons"), [] {
        AddonManager::current()->disable_old_addons();
        MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      });

      dialog->add_button(_("Ignore (not advised)"), [] {
        MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      });

      dialog->add_button(_("Leave editor"), [this] {
        quit_request = true;
      });

      MenuManager::instance().set_dialog(std::move(dialog));
    }
    else
#endif
    if (g_config->editor_remember_last_level &&
        !g_config->editor_last_edited_level.empty())
    {
      m_project->set_world(std::move(
        World::from_directory(FileSystem::dirname(g_config->editor_last_edited_level))));
      set_level(FileSystem::basename(g_config->editor_last_edited_level));
    }
    else
    {
      set_level(nullptr, true);
      g_config->editor_last_edited_level = "";
    }
  }
  m_toolbox_widget->setup();
  m_layers_widget->setup();

  if (!g_config->max_viewport && g_config->editor_max_viewport)
    VideoSystem::current()->get_viewport().force_full_viewport(true);

  // Reactivate the editor after level test.
  reactivate_after_level_test();
}

void
Editor::deactivate()
{
  m_enabled = false;
  
  if (!m_testing_level)
  {
    MouseCursor::current()->set_visible(true);
  }
}

void
Editor::reactivate_after_menu_close()
{
  if (!m_enabled)
  {
    // It's possible that the editor is being re-activated due to exiting a menu,
    // possibly one related to an object option.
    GameObject* selected_object = m_selected_object.get();
    if (selected_object)
    {
      selected_object->after_editor_set();
      selected_object->check_state();
    }
  }
  
  m_enabled = true;

  m_ctrl_pressed = m_alt_pressed = false;

  // any mouse events from earlier (i.e. in menu, testing) dont pass through
  // the editor in those states, so as a lazy hack, let's just get the mouse
  // position.

  float x, y;
  SDL_GetMouseState(&x, &y);
  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(x, y);
}

void
Editor::reactivate_after_level_test()
{
  if (!m_testing_level)
    return;

  m_testing_level = false;

  m_enabled = true;

  Tile::draw_editor_images = true;

  m_project->reactivate();

  MenuManager::instance().clear_menu_stack();
  SoundManager::current()->stop_music();

  m_toolbox_widget->update_mouse_icon();
}

void
Editor::on_window_resize()
{
  for(const auto& widget: m_widgets)
  {
    widget->on_window_resize();
  }
}

bool
Editor::has_focus() const
{
  if (!m_enabled || !m_project->is_level_loaded())
    return false;

  const auto& menu_manager = MenuManager::instance();
  if (menu_manager.is_active() || menu_manager.has_dialog())
    return false;

  auto console = Console::current();
  if (console && console->hasFocus())
    return false;

  return true;
}

void
Editor::event(const SDL_Event& ev)
{
  if (!has_focus())
    return;

  for(const auto& control : m_controls)
    if (control->event(ev))
      return;

  auto sector = m_project->get_sector();
  auto& camera = sector->get_camera();

  try
  {
    if (ev.type == SDL_EVENT_MOUSE_MOTION)
    {
      m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);

      // If properties sidebar controls are active and the mouse is hovering over the sidebar,
      // do not propagate mouse motion to the editor or its widgets.
      if (!m_controls.empty() && Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f).contains(m_mouse_pos))
        return;
    }
    else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
      switch (ev.button.button)
      {
        case SDL_BUTTON_X1:
          undo();
          break;
        case SDL_BUTTON_X2:
          redo();
          break;
      }
    } else {
      // If properties sidebar controls are active and the mouse is hovering over the sidebar,
      // do not propagate mouse events to the editor or its widgets.
      if (!m_controls.empty() &&
          (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
           ev.type == SDL_EVENT_MOUSE_BUTTON_UP ||
           ev.type == SDL_EVENT_MOUSE_WHEEL) &&
          Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f).contains(m_mouse_pos))
      {
        return;
      }

      if (ev.type == SDL_EVENT_KEY_DOWN)
      {
        m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
        m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
        m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;

        if (m_ctrl_pressed)
          m_scroll_speed = 16.0f;
        else if (ev.key.mod & SDL_KMOD_RSHIFT)
          m_scroll_speed = 96.0f;

        if (ev.key.key == SDLK_F6)
        {
          Compositor::s_render_lighting = !Compositor::s_render_lighting;
          return;
        }
        else if (m_ctrl_pressed)
        {
          switch (ev.key.key)
          {
            case SDLK_T:
              if (m_shift_pressed && m_alt_pressed)
              {
                test_level(m_last_test_pos);
                break;
              }

              if (m_shift_pressed)
                m_last_test_pos = std::pair<std::string, Vector>(sector->get_name(), m_overlay_widget->get_sector_pos());
              else
                m_last_test_pos = std::nullopt;

              test_level(m_last_test_pos);
              break;
            case SDLK_S:
              m_project->save_level();
              break;
            case SDLK_Z:
              undo();
              break;
            case SDLK_Y:
              redo();
              break;
            case SDLK_H:
              m_show_draggables = !m_show_draggables;
              if (!m_show_draggables)
                m_show_draggables_hint.start(6.7f);
              break;
            case SDLK_X:
              m_toolbar_widget->toggle_tile_object_mode();
              break;
            case SDLK_PAGEUP:
              m_toolbox_widget->switch_current_group(-1);
              break;
            case SDLK_PAGEDOWN:
              m_toolbox_widget->switch_current_group(1);
              break;
            case SDLK_PLUS: // Zoom in
            case SDLK_EQUALS:
            case SDLK_KP_PLUS:
              m_key_zoomed = true;
              m_new_scale = camera.get_current_scale() + CAMERA_ZOOM_SENSITIVITY;
              break;
            case SDLK_MINUS: // Zoom out
            case SDLK_KP_MINUS:
              m_key_zoomed = true;
              m_new_scale = camera.get_current_scale() - CAMERA_ZOOM_SENSITIVITY;
              break;
            case SDLK_D: // Reset zoom
              m_new_scale = 1.f;
              break;
            default:
              break;
          }
        }
      }
      else if (ev.type == SDL_EVENT_KEY_UP)
      {
        m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
        m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
        m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;

        if (!m_ctrl_pressed && !(ev.key.mod & SDL_KMOD_RSHIFT))
          m_scroll_speed = 32.0f;
      }
      else if (ev.type == SDL_EVENT_PEN_BUTTON_DOWN)
      {
        m_pen_down = true;
      }
      else if (ev.type == SDL_EVENT_PEN_BUTTON_UP)
      {
        m_pen_down = false;
      }
      else if (ev.type == SDL_EVENT_MOUSE_WHEEL && !m_toolbox_widget->has_mouse_focus() && !m_layers_widget->has_mouse_focus())
      {
#if SDL_VERSION_ATLEAST(3, 2, 12)
        float wheel_x = g_config->precise_scrolling ? ev.wheel.x : ev.wheel.integer_x;
        float wheel_y = g_config->precise_scrolling ? ev.wheel.y : ev.wheel.integer_y;
#else
        float wheel_x = ev.wheel.x;
        float wheel_y = ev.wheel.y;
#endif
        if (g_config->invert_wheel_x) wheel_x *= -1.f;
        if (g_config->invert_wheel_y) wheel_y *= -1.f;
        // Scroll or zoom with mouse wheel, if the mouse is not over the toolbox.
        // The toolbox does scrolling independently from the main area.
        if (m_ctrl_pressed)
          m_new_scale = camera.get_current_scale() + static_cast<float>(wheel_y) * CAMERA_ZOOM_SENSITIVITY;
        else
          scroll({ static_cast<float>((m_shift_pressed ? wheel_y * (g_config->editor_invert_shift_scroll ? -1 : 1) : wheel_x) * 40),
                   static_cast<float>((m_shift_pressed ? wheel_x : wheel_y) * -40) });
      }
    }

    BIND_SECTOR(*m_project->get_sector());

    if (m_toolbar_widget->event(ev))
      return;

    for (const auto& widget : m_widgets)
      if (widget->event(ev))
        break;
  }
  catch(const std::exception& err)
  {
    log_warning << "error while processing Editor::event(): " << err.what() << std::endl;
  }
}

void
Editor::update_node_iterators()
{
  m_overlay_widget->update_node_iterators();
}

void
Editor::delete_markers()
{
  m_overlay_widget->delete_markers();
}

void
Editor::sort_layers()
{
  m_layers_widget->sort_layers();
}

void
Editor::select_tilegroup(int id)
{
  // dumb hack around dumb design...
  if (m_toolbox_widget->get_tilebox().get_input_type() != InputType::TILE)
    m_toolbar_widget->toggle_tile_object_mode();

  m_toolbox_widget->select_tilegroup(id);
}

void
Editor::select_last_tilegroup()
{
  m_toolbox_widget->select_last_tilegroup();
}

void
Editor::change_tileset()
{
  auto level = m_project->get_level();
  auto level_tileset = level->get_tileset();
  m_project->set_tileset(TileManager::current()->get_tileset(level_tileset));
  m_toolbox_widget->get_tilebox().set_input_type(InputType::TILE);
  for (const auto& sector : level->get_sectors()) {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>()) {
      tilemap.set_tileset(m_project->get_tileset());
    }
  }
  m_toolbox_widget->get_tilebox().select_tilegroup(0);
}

void
Editor::select_objectgroup(int id)
{
  // dumb hack around dumb design...
  if (m_toolbox_widget->get_tilebox().get_input_type() != InputType::OBJECT)
    m_toolbar_widget->toggle_tile_object_mode();

  m_toolbox_widget->select_objectgroup(id);
}

void
Editor::select_last_objectgroup()
{
  m_toolbox_widget->select_last_objectgroup();
}

const std::vector<ObjectGroup>&
Editor::get_objectgroups() const
{
  return m_toolbox_widget->get_tilebox().get_object_info().m_groups;
}

void
Editor::retoggle_undo_tracking()
{
  auto level = m_project->get_level();
  m_toolbar_widget->set_undo_disabled(true);
  m_toolbar_widget->set_redo_disabled(true);
  // Toggle undo tracking for all sectors.
  for (const auto& sector : level->get_sectors())
    sector->toggle_undo_tracking(g_config->editor_undo_tracking);
}

void
Editor::undo_stack_cleanup()
{
  auto level = m_project->get_level();
  // Set the undo stack size and perform undo stack cleanup on all sectors.
  for (const auto& sector : level->get_sectors())
  {
    sector->set_undo_stack_size(g_config->editor_undo_stack_size);
    sector->undo_stack_cleanup();
  }
}

void
Editor::undo()
{
  auto sector = m_project->get_sector();
  BIND_SECTOR(*sector);
  sector->undo();
  m_layers_widget->update_current_tip();
}

void
Editor::redo()
{
  auto sector = m_project->get_sector();
  BIND_SECTOR(*sector);
  sector->redo();
  m_layers_widget->update_current_tip();
}

IntegrationStatus
Editor::get_status() const
{
  auto level = m_project->get_level();

  IntegrationStatus status;
  status.m_details.push_back("In Editor");
  if (!g_config->hide_editor_levelnames && level)
  {
    std::string level_type = (level->is_worldmap() ? "worldmap" : "level");
    std::string status_text = "Editing " + level_type + ": " + level->get_name();

    status.m_details.push_back(status_text);
  }
  return status;
}

bool
Editor::get_properties_panel_visible() const
{
  return !m_controls.empty() && g_config->editor_show_properties_sidebar;
}

void
Editor::add_control(const std::string& name, std::unique_ptr<InterfaceControl> new_control, const std::string& description)
{
  assert(new_control);
  if (!g_config->editor_show_properties_sidebar)
    return;

  float height = 35.f;
  for (const auto& control : m_controls)
    height = std::max(height, control->get_rect().get_bottom() + 5.f);

  auto control_rect = new_control->get_rect();
  Rectf target_rect;
  if (control_rect.get_width() == 0.f || control_rect.get_height() == 0.f)
  {
    target_rect = Rectf(100.f, height, 200.f - 1.0f, height + 20.f);
  }
  else
  {
    target_rect = Rectf(control_rect.get_left(), height,
                        control_rect.get_right(), height + control_rect.get_height());
  }
  new_control->set_rect(target_rect);

  auto dimensions = Rectf(3.f, height, 100.f, height + 20.f);
  new_control->m_label = std::make_unique<InterfaceLabel>(dimensions, std::move(name), std::move(description));
  m_controls.push_back(std::move(new_control));
}

void
Editor::select_object(GameObject* object)
{
  m_controls.clear();

  if (!object || !g_config->editor_show_properties_sidebar)
  {
    m_selected_object = 0;
    return;
  }
  m_selected_object = object;

  ObjectSettings os = object->get_settings();
  for (const auto& option : os.get_options())
  {
    if ((option->get_flags() & OPTION_HIDDEN) && !(option->get_flags() & OPTION_VISIBLE_PROPERTIES))
      continue;

    auto control = option->create_interface_control();
    if (!control)
      continue;

    control->m_on_activate_callbacks.emplace_back([object]() {
        object->save_state();
      });
    control->m_on_change_callbacks.emplace_back([object]() {
        // TODO: Updating the object doesn't work every time.
        // Investigate why this is the case!
        object->after_editor_set();
        object->check_state();
      });
    add_control(option->get_text(), std::move(control), option->get_description());
  }
}
