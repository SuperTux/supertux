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

#include <fstream>
#include <sstream>
#include <limits>
#include <unordered_map>

#ifdef EMSCRIPTEN
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

static const float CAMERA_MIN_ZOOM = 0.5f;
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
    return self && !self->m_leveltested && self->m_after_setup;
  }
}

Editor::Editor() :
  m_level(),
  m_world(),
  m_levelfile(),
  m_autosave_levelfile(),
  m_quit_request(false),
  m_newlevel_request(false),
  m_reload_request(false),
  m_reactivate_request(false),
  m_deactivate_request(false),
  m_save_request(false),
  m_save_request_filename(""),
  m_save_request_switch(false),
  m_test_request(false),
  m_particle_editor_request(false),
  m_test_pos(),
  m_temp_level(true),
  m_particle_editor_filename(),
  m_ctrl_pressed(false),
  m_shift_pressed(false),
  m_sector(),
  m_levelloaded(false),
  m_leveltested(false),
  m_after_setup(false),
  m_tileset(nullptr),
  m_has_deprecated_tiles(false),
  m_widgets(),
  m_controls(),
  m_undo_widget(),
  m_redo_widget(),
  m_grid_size_widget(),
  m_play_widget(),
  m_save_widget(),
  m_overlay_widget(),
  m_toolbox_widget(),
  m_layers_widget(),
  m_enabled(false),
  m_bgr_surface(Surface::from_file("images/engine/menu/bg_editor.png")),
  m_time_since_last_save(0.f),
  m_scroll_speed(32.0f),
  m_new_scale(0.f),
  m_mouse_pos(0.f, 0.f),
  m_layers_widget_needs_refresh(false)
{
  auto toolbox_widget = std::make_unique<EditorToolboxWidget>(*this);
  auto layers_widget = std::make_unique<EditorLayersWidget>(*this);
  auto overlay_widget = std::make_unique<EditorOverlayWidget>(*this);

  m_toolbox_widget = toolbox_widget.get();
  m_layers_widget = layers_widget.get();
  m_overlay_widget = overlay_widget.get();

  m_widgets.push_back(std::move(toolbox_widget));
  m_widgets.push_back(std::move(layers_widget));
  m_widgets.push_back(std::move(overlay_widget));

  auto grid_size_widget = std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/grid_button.png",
    Vector(64, 0),
    [this] {
      auto& snap_grid_size = g_config->editor_selected_snap_grid_size;
      if (snap_grid_size == 0)
      {
        if(!g_config->editor_render_grid)
        {
          snap_grid_size = 3;
        }
        g_config->editor_render_grid = !g_config->editor_render_grid;
      }
      else
        snap_grid_size--;
    });
  
  grid_size_widget->set_help_text(_("Change / Toggle grid size"));
  m_grid_size_widget = grid_size_widget.get();

  m_widgets.insert(m_widgets.begin() + 2, std::move(grid_size_widget));

  auto play_button = std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/play_button.png",
    Vector(96, 0), [this] { m_test_request = true; });
  play_button->set_help_text(_("Test level"));

  m_play_widget = play_button.get();

  m_widgets.insert(m_widgets.begin() + 3, std::move(play_button));

  auto save_button = std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/save.png",
    Vector(128, 0), [this] { 
	  bool saved = save_level();
	  auto notif = std::make_unique<Notification>("save_level_notif");
	  notif->set_text(saved ? _("Level saved!") : _("Level failed to save."));
	  MenuManager::instance().set_notification(std::move(notif));
	}
  );
  save_button->set_help_text(_("Save level"));

  m_save_widget = save_button.get();

  m_widgets.insert(m_widgets.begin() + 4, std::move(save_button));
  
  auto mode_button = std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/toggle_tile_object_mode.png",
    Vector(160, 0), [this] {
	  toggle_tile_object_mode();
    }
  );
  mode_button->set_help_text(_("Toggle between object and tile mode"));

  m_widgets.insert(m_widgets.begin() + 5, std::move(mode_button));

  auto mouse_select_button = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/arrow.png", Vector(192, 0), [this]() {
      m_toolbox_widget->set_mouse_tool();
    }
  );
  mouse_select_button->set_help_text(_("Toggle between add and remove mode"));
  
  /**
   *  ============= Tools only applicable for Tile mode =====================
   */
  auto select_mode_mouse_button = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/select-mode0.png", Vector(224, 0), [this] {
      m_toolbox_widget->set_tileselect_select_mode(0);
    });
  select_mode_mouse_button->set_help_text(_("Draw mode (The current tool applies to the tile under the mouse)"));
  select_mode_mouse_button->set_visible_in_object_mode(false);
  select_mode_mouse_button->set_visible(false);
  
  auto select_mode_area_button = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/select-mode1.png", Vector(256, 0), [this] {
      m_toolbox_widget->set_tileselect_select_mode(1);
    });
  select_mode_area_button->set_help_text(_("Box draw mode (The current tool applies to an area / box drawn with the mouse)"));
  select_mode_area_button->set_visible_in_object_mode(false);
  select_mode_area_button->set_visible(false);
  
  auto select_mode_fill_button = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/select-mode2.png", Vector(288, 0), [this] {
      m_toolbox_widget->set_tileselect_select_mode(2);
    });
  select_mode_fill_button->set_help_text(_("Fill mode (The current tool applies to the empty area in the enclosed space that was clicked)"));
  select_mode_fill_button->set_visible_in_object_mode(false);
  select_mode_fill_button->set_visible(false);
  
  auto select_mode_same_button = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/select-mode3.png", Vector(320, 0), [this] {
      m_toolbox_widget->set_tileselect_select_mode(3);
    });
  select_mode_same_button->set_help_text(_("Replace mode (The current tool applies to all tiles that are the same tile as the one under the mouse)"));
  select_mode_same_button->set_visible_in_object_mode(false);
  select_mode_same_button->set_visible(false);

  /**
   *  ============= Tile tools end / Object tools begin =====================
   */
  auto select_mode = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/move-mode0.png", Vector(224, 0), [this] {
      m_toolbox_widget->set_tileselect_move_mode(0);
  });
  select_mode->set_help_text(_("Select mode (Clicking selects the object under the mouse)"));
  select_mode->set_visible_in_tile_mode(false);
  select_mode->set_visible(false);

  auto duplicate_mode = std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/move-mode1.png", Vector(256, 0), [this] {
      m_toolbox_widget->set_tileselect_move_mode(1);
  });
  duplicate_mode->set_help_text(_("Duplicate mode (Clicking duplicates the object under the mouse)"));
  duplicate_mode->set_visible_in_tile_mode(false);
  duplicate_mode->set_visible(false);

  m_widgets.insert(m_widgets.begin() + 6, std::move(mouse_select_button));
  m_widgets.insert(m_widgets.begin() + 7, std::move(select_mode_mouse_button));
  m_widgets.insert(m_widgets.begin() + 8, std::move(select_mode_area_button));
  m_widgets.insert(m_widgets.begin() + 9, std::move(select_mode_fill_button));
  m_widgets.insert(m_widgets.begin() + 10, std::move(select_mode_same_button));
  m_widgets.insert(m_widgets.begin() + 11, std::move(select_mode));
  m_widgets.insert(m_widgets.begin() + 12, std::move(duplicate_mode));

  // auto code_widget = std::make_unique<EditorToolbarButtonWidget>(
  //   "images/engine/editor/select-mode3.png", Vector(320, 0), [this] {
  //     std::ostringstream level_ostream;
  //     Writer output_writer(level_ostream);
  //     m_level->save(output_writer);
  //     auto level_content = level_ostream.str();
  //     MenuManager::instance().push_menu(std::make_unique<ScriptMenu>(&level_content));
  //     log_warning << level_content << std::endl;
  //   });
  // m_widgets.insert(m_widgets.begin() + 10, std::move(code_widget));
}

Editor::~Editor()
{
}

void
Editor::level_from_nothing()
{
	m_level = std::make_unique<Level>(false);
	m_level->m_name = "Supertux Level";
	m_level->m_tileset = "images/tiles.strf";
	auto sector = SectorParser::from_nothing(*m_level);
	sector->set_name(DEFAULT_SECTOR_NAME);
	m_level->add_sector(std::move(sector));
	m_level->initialize();
	//m_reload_request = true;
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

  if (m_levelloaded) {
    for(const auto& widget : m_widgets) {
      widget->draw(context);
    }
	
	m_overlay_widget->draw_tilemap_outer_shading(context);
	m_overlay_widget->draw_tilemap_border(context);
    
	if (m_controls.size() != 0)
	{
	  context.color().draw_filled_rect(Rectf(0.0f, 0.0f, SCREEN_WIDTH, 32.0f),
										 Color(0.2f, 0.2f, 0.2f, 0.5f), LAYER_GUI - 6);

	  context.color().draw_filled_rect(Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f),
										 Color(0.2f, 0.2f, 0.2f, 0.5f), LAYER_GUI - 6);

	  for(const auto& control : m_controls)
	  {
	    control->draw(context);
	  }
	}

    // If camera scale must be changed, change it here.
    if (m_new_scale != 0.f)
    {
      // Do not clamp, as to prevent pointless calls to EditorOverlayWidget::update_pos().
      if (m_new_scale >= CAMERA_MIN_ZOOM && m_new_scale <= CAMERA_MAX_ZOOM)
      {
        Camera& camera = m_sector->get_camera();
        const bool zooming_in = camera.get_current_scale() < m_new_scale;

        camera.set_scale(m_new_scale);

        // When zooming in, focus on the position of the mouse.
        if (zooming_in)
          camera.move((m_mouse_pos - Vector(static_cast<float>(SCREEN_WIDTH - 128),
                                            static_cast<float>(SCREEN_HEIGHT - 32)) / 2.f) / CAMERA_ZOOM_FOCUS_PROGRESSION);

        keep_camera_in_bounds();
      }
      m_new_scale = 0.f;
    }

    m_sector->pause_camera_interpolation();

    // Avoid drawing the sector if we're about to test it, as there is a dangling pointer
    // issue with the PlayerStatus.
    if (!m_leveltested)
      m_sector->draw(context);

    context.color().draw_filled_rect(context.get_rect(),
                                     Color(0.0f, 0.0f, 0.0f),
                                     0.0f, std::numeric_limits<int>::min());
  } else {
    context.color().draw_surface_scaled(m_bgr_surface,
                                        context.get_rect(),
                                        -100);
  }

  MouseCursor::current()->draw(context);
}

void
Editor::update(float dt_sec, const Controller& controller)
{
  // Auto-save (interval).
  if (m_level && !m_temp_level) {
    m_time_since_last_save += dt_sec;
    if (m_time_since_last_save >= static_cast<float>(std::max(
        g_config->editor_autosave_frequency, 1)) * 60.f) {
      m_time_since_last_save = 0.f;
      std::string backup_filename = get_autosave_from_levelname(m_levelfile);
      std::string directory = get_level_directory();

      // Set the test level file even though we're not testing, so that
      // if the user quits the editor without ever testing, it'll delete
      // the autosave file anyways.
      m_autosave_levelfile = FileSystem::join(directory, backup_filename);
      try
      {
        m_level->save(m_autosave_levelfile);
      }
      catch(const std::exception& e)
      {
        log_warning << "Couldn't autosave: " << e.what() << '\n';
      }
    }
  } else {
    m_time_since_last_save = 0.f;
  }

  // Pass all requests.
  if (m_reload_request) {
    reload_level();
  }

  if (m_quit_request) {
    quit_editor();
  }

  if (m_newlevel_request) {
    // Create new level.
  }

  if (m_reactivate_request) {
    m_enabled = true;
    m_reactivate_request = false;
  }

  if (m_save_request) {
    save_level(m_save_request_filename, m_save_request_switch);
    m_enabled = true;
    m_save_request = false;
    m_save_request_filename = "";
    m_save_request_switch = false;
  }

  if (m_test_request) {
    m_test_request = false;
    MouseCursor::current()->set_icon(nullptr);
    test_level(m_test_pos);
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

  if (m_deactivate_request) {
    m_enabled = false;
    m_deactivate_request = false;
    return;
  }

  // Update other components.
  if (m_levelloaded && !m_leveltested) {
    BIND_SECTOR(*m_sector);

    for (auto& object : m_sector->get_objects()) {
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

    for(const auto& control : m_controls)
    {
      control->update(dt_sec);
    }

    // Now that all widgets have been updated, which should have relinquished
    // pointers to objects marked for deletion, we can actually delete them.
    for (auto& sector : m_level->get_sectors())
      sector->flush_game_objects();

    update_keyboard(controller);
  }
}

void
Editor::remove_autosave_file()
{
  if (m_temp_level)
    return;
  
  // Clear the auto-save file.
  if (!m_autosave_levelfile.empty())
  {
    // Try to remove the test level using the PhysFS file system
    if (physfsutil::remove(m_autosave_levelfile) != 0)
    {
      // This file is not inside any PhysFS mounts,
      // try to remove this using normal file system
      // methods.
      FileSystem::remove(m_autosave_levelfile);
    }
  }
}

bool
Editor::save_level(const std::string& filename, bool switch_file)
{
  if (m_temp_level)
    return false;

  auto file = !filename.empty() ? filename : m_levelfile;

  if (switch_file)
    m_levelfile = filename;

  for (const auto& sector : m_level->m_sectors)
  {
    sector->on_editor_save();
  }
  m_level->save(m_world ? FileSystem::join(m_world->get_basedir(), file) : file);
  m_time_since_last_save = 0.f;
  remove_autosave_file();
  return true;
}

std::string
Editor::get_level_directory() const
{
  std::string basedir;
  if (m_world != nullptr)
  {
    basedir = m_world->get_basedir();
    if (basedir == "./")
    {
      basedir = PHYSFS_getRealDir(m_levelfile.c_str());
    }
  }
  else
  {
    basedir = FileSystem::dirname(m_levelfile);
  }
  return basedir;
}

void
Editor::test_level(const std::optional<std::pair<std::string, Vector>>& test_pos)
{
  Tile::draw_editor_images = false;
  Compositor::s_render_lighting = true;
 
  // Until I get testing to not clobber the editor level, display a message.
  if (m_temp_level)
  {
    std::string message = _("You cannot test an unsaved level at the moment.\n\n"
		"Please save your level before testing.");

    Dialog::show_message(message);
	return;
  }

  m_leveltested = true;
  if ((m_level && m_levelfile.empty()) || m_levelfile == "")
  {
    GameManager::current()->start_level(m_level.get(), test_pos);
	return;
  }
  
  std::string backup_filename = get_autosave_from_levelname(m_levelfile);
  std::string directory = get_level_directory();

  // This is jank to get an owned World pointer, GameManager/World
  // could probably need a refactor to handle this better.
  std::unique_ptr<World> owned_world;
  World* current_world = m_world.get();
  if (!current_world) {
    owned_world = World::from_directory(directory);
    current_world = owned_world.get();
  }

  m_autosave_levelfile = FileSystem::join(directory, backup_filename);
  m_level->save(m_autosave_levelfile);
  m_time_since_last_save = 0.f;

  if (!m_level->is_worldmap())
  {
    // TODO: After LevelSetScreen is removed, this should return a boolean indicating whether load was successful.
    //       If not, call reactivate().
    GameManager::current()->start_level(*current_world, backup_filename, test_pos);
  }
  else if (!GameManager::current()->start_worldmap(*current_world, m_autosave_levelfile, test_pos))
  {
    reactivate();
  }
}

void
Editor::open_level_directory()
{
  if (m_temp_level)
    return;
  m_level->save(FileSystem::join(get_level_directory(), m_levelfile));
  auto path = FileSystem::join(PHYSFS_getWriteDir(), get_level_directory());
  FileSystem::open_path(path);
}

void
Editor::scroll(const Vector& velocity)
{
  if (!m_levelloaded) return;

  m_sector->get_camera().move(velocity / m_sector->get_camera().get_current_scale());
  keep_camera_in_bounds();
}

void
Editor::keep_camera_in_bounds()
{
  Camera& camera = m_sector->get_camera();
  constexpr float offset = 80.f;
  float controls_offset_x = m_controls.size() != 0 ? -200.f : 0.f;
  float controls_offset_y = m_controls.size() != 0 ? -32.f : 0.f;
  camera.keep_in_bounds(Rectf(-offset + controls_offset_x, -offset + controls_offset_y,
                              std::max(0.0f, m_sector->get_editor_width() + 128.f / camera.get_current_scale()) + offset,
                              std::max(0.0f, m_sector->get_editor_height() + 32.f / camera.get_current_scale()) + offset));

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
  if (!m_enabled) {
    return;
  }

  if (MenuManager::instance().is_active() || MenuManager::instance().has_dialog())
    return;

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
  if (controller.hold(Control::LEFT)) {
    scroll({ -m_scroll_speed, 0.0f });
  }

  if (controller.hold(Control::RIGHT)) {
    scroll({ m_scroll_speed, 0.0f });
  }

  if (controller.hold(Control::UP)) {
    scroll({ 0.0f, -m_scroll_speed });
  }

  if (controller.hold(Control::DOWN)) {
    scroll({ 0.0f, m_scroll_speed });
  }
}

void
Editor::load_sector(const std::string& name)
{
  Sector* sector = m_level->get_sector(name);
  if (!sector) {
    sector = m_level->get_sector(0);
  }

  sector->set_undo_stack_size(g_config->editor_undo_stack_size);
  sector->toggle_undo_tracking(g_config->editor_undo_tracking);

  set_sector(sector);
}

void
Editor::set_sector(Sector* sector)
{
  if (!sector) return;

  m_sector = sector;
  m_sector->activate(DEFAULT_SPAWNPOINT_NAME);

  { // Initialize badguy sprites and perform other GameObject related tasks.
    BIND_SECTOR(*m_sector);
    for(auto& object : m_sector->get_objects()) {
      object->after_editor_set();
    }
  }

  m_layers_widget->refresh();
}

void
Editor::delete_current_sector()
{
  if (m_level->m_sectors.size() <= 1) {
    log_fatal << "Deleting the last sector is not allowed." << std::endl;
  }

  for (auto i = m_level->m_sectors.begin(); i != m_level->m_sectors.end(); ++i) {
    if ( i->get() == get_sector() ) {
      m_level->m_sectors.erase(i);
      break;
    }
  }

  set_sector(m_level->m_sectors.front().get());
  m_reactivate_request = true;
}

void
Editor::set_level(std::unique_ptr<Level> level, bool reset)
{
  std::string sector_name = DEFAULT_SECTOR_NAME;
  Vector translation(0.0f, 0.0f);
  
  m_temp_level = (level == nullptr);

  if (!reset && m_sector) {
    translation = m_sector->get_camera().get_translation();
    sector_name = m_sector->get_name();
  }

  m_reload_request = false;
  m_enabled = true;

  if (reset) {
    m_toolbox_widget->get_tilebox().set_input_type(EditorTilebox::InputType::NONE);
  }

  m_levelloaded = true;

  if (level != nullptr) {
    // Reload level.
	m_level = std::move(level);
  }
  else
  {
    level_from_nothing();
  }
  
  if (reset) {
    m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  }

  load_sector(sector_name);

  if (m_sector != nullptr)
  {
    m_sector->get_camera().set_mode(Camera::Mode::MANUAL);

    if (!reset) {
      m_sector->get_camera().set_translation(translation);
    }
  }

  m_layers_widget->refresh_sector_text();
  m_toolbox_widget->update_mouse_icon();
  m_overlay_widget->on_level_change();

  if (!reset) return;

  // Warn the user if any deprecated tiles are used throughout the level
  check_deprecated_tiles();
  if (m_has_deprecated_tiles)
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
  ReaderMapping::s_translations_enabled = false;
  try
  {
    set_level(LevelParser::from_file(m_world ?
                                     FileSystem::join(m_world->get_basedir(), m_levelfile) : m_levelfile,
                                     StringUtil::has_suffix(m_levelfile, ".stwm"),
                                     true));
  }
  catch (const std::exception& err)
  {
    log_warning << "Error loading level '" << m_levelfile << "' in editor: " << err.what() << std::endl;
    reset_level();
    return;
  }
  ReaderMapping::s_translations_enabled = true;

  retoggle_undo_tracking();
  undo_stack_cleanup();

  // Autosave files : Once the level is loaded, make sure
  // to use the regular file.
  m_levelfile = get_levelname_from_autosave(m_levelfile);
  m_autosave_levelfile = FileSystem::join(get_level_directory(),
                                          get_autosave_from_levelname(m_levelfile));
}

void
Editor::reset_level()
{
  m_levelloaded = false;
  m_level.reset();
  m_world.reset();
  m_levelfile.clear();
  m_sector = nullptr;

  m_reload_request = false;

  MouseCursor::current()->set_icon(nullptr);
  MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
}

void
Editor::quit_editor()
{
  m_quit_request = false;

  auto quit = [this] ()
  {
    remove_autosave_file();

    // Quit level editor.
    m_world = nullptr;
    m_levelfile = "";
    m_levelloaded = false;
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
  };

  check_unsaved_changes([quit] {
    quit();
  });
}

void
Editor::check_unsaved_changes(const std::function<void ()>& action)
{
  if (!m_levelloaded || m_temp_level)
  {
    action();
    return;
  }

  bool has_unsaved_changes = !g_config->editor_undo_tracking;
  if (!has_unsaved_changes)
  {
    for (const auto& sector : m_level->m_sectors)
    {
      if (sector->has_object_changes())
      {
        has_unsaved_changes = true;
        break;
      }
    }
  }

  if (has_unsaved_changes)
  {
    m_enabled = false;
    auto dialog = std::make_unique<Dialog>();
    dialog->set_text(g_config->editor_undo_tracking ? _("This level contains unsaved changes, do you want to save?") :
                                                      _("This level may contain unsaved changes, do you want to save?"));
    dialog->add_default_button(_("Yes"), [this, action] {
      check_save_prerequisites([this, action] {
        save_level();
        action();
        m_enabled = true;
      });
    });
    dialog->add_button(_("No"), [this, action] {
      action();
	  set_level(nullptr, true);
      m_enabled = true;
    });
    dialog->add_button(_("Cancel"), [this] {
      m_enabled = true;
    });
    MenuManager::instance().set_dialog(std::move(dialog));
  }
  else
  {
    action();
  }
}

void
Editor::check_deprecated_tiles(bool focus)
{
  // Check for any deprecated tiles, used throughout the entire level
  m_has_deprecated_tiles = false;
  for (const auto& sector : m_level->get_sectors())
  {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>())
    {
      int pos = -1;
      for (const uint32_t& tile_id : tilemap.get_tiles())
      {
        pos++;
        if (m_tileset->get(tile_id).is_deprecated())
        {
          // Focus on deprecated tile
          if (focus)
          {
            set_sector(sector.get());
            m_layers_widget->set_selected_tilemap(&tilemap);

            const int width = tilemap.get_width();
            m_sector->get_camera().set_translation_centered(Vector(pos % width, pos / width) * 32.f);
            keep_camera_in_bounds();
          }

          m_has_deprecated_tiles = true;
          return;
        }
      }
    }
  }
}

void
Editor::convert_tiles_by_file(const std::string& file)
{
  std::unordered_map<int, int> tiles;

  try
  {
    IFileStream in(file);
    if (!in.good())
      throw std::runtime_error("Error opening file stream!");

    int a, b;
    std::string delimiter;
    while (in >> a >> delimiter >> b)
    {
      if (delimiter != "->")
        throw std::runtime_error("Expected '->' delimiter!");

      tiles[a] = b;
    }
  }
  catch (std::exception& err)
  {
    log_warning << "Couldn't parse conversion file '" << file << "': " << err.what() << std::endl;
    return;
  }

  for (const auto& sector : m_level->get_sectors())
  {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>())
    {
      tilemap.save_state();
      // Can't use change_all(), if there's like `1 -> 2`and then
      // `2 -> 3`, it'll do a double replacement
      for (int x = 0; x < tilemap.get_width(); x++)
      {
        for (int y = 0; y < tilemap.get_height(); y++)
        {
          auto tile = tilemap.get_tile_id(x, y);
          try
          {
            tilemap.change(x, y, tiles.at(tile));
          }
          catch (std::out_of_range&)
          {
            // Expected for tiles that don't need to be replaced
          }
        }
      }
      tilemap.check_state();
    }
  }
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
  if (!m_levelloaded) {

#if 0
    if (AddonManager::current()->is_old_addon_enabled()) {
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
    } else
#endif
    {
      MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
    }
	set_level(nullptr, true);
  }
  m_toolbox_widget->setup();
  m_layers_widget->setup();

  // Reactivate the editor after level test.
  reactivate();
}

void
Editor::reactivate()
{
  // Reactivate the editor after level test.
  if (!m_leveltested)
    return;

  m_leveltested = false;
  Tile::draw_editor_images = true;
  m_level->reactivate();

  m_sector->activate(Vector(0,0));

  MenuManager::instance().clear_menu_stack();
  SoundManager::current()->stop_music();

  m_deactivate_request = false;
  m_enabled = true;
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

void
Editor::event(const SDL_Event& ev)
{
  if (!m_enabled || !m_levelloaded) return;

  for(const auto& control : m_controls)
    if (control->event(ev))
      return;

  try
  {
    if (ev.type == SDL_KEYDOWN)
    {
      m_ctrl_pressed = ev.key.keysym.mod & KMOD_CTRL;
      m_shift_pressed = ev.key.keysym.mod & KMOD_SHIFT;

      if (m_ctrl_pressed)
        m_scroll_speed = 16.0f;
      else if (ev.key.keysym.mod & KMOD_RSHIFT)
        m_scroll_speed = 96.0f;

      if (ev.key.keysym.sym == SDLK_F6)
      {
        Compositor::s_render_lighting = !Compositor::s_render_lighting;
        return;
      }
      else if (m_ctrl_pressed)
      {
        switch (ev.key.keysym.sym)
        {
          case SDLK_t:
            test_level(std::nullopt);
            break;
          case SDLK_s:
            save_level();
            break;
          case SDLK_z:
            undo();
            break;
          case SDLK_y:
            redo();
            break;
		  case SDLK_x:
            toggle_tile_object_mode();
		    break;
          case SDLK_PLUS: // Zoom in
          case SDLK_KP_PLUS:
            m_new_scale = m_sector->get_camera().get_current_scale() + CAMERA_ZOOM_SENSITIVITY;
            break;
          case SDLK_MINUS: // Zoom out
          case SDLK_KP_MINUS:
            m_new_scale = m_sector->get_camera().get_current_scale() - CAMERA_ZOOM_SENSITIVITY;
            break;
          case SDLK_d: // Reset zoom
            m_new_scale = 1.f;
            break;
        }
      }
    }
    else if (ev.type == SDL_KEYUP)
    {
      m_ctrl_pressed = ev.key.keysym.mod & KMOD_CTRL;
      m_shift_pressed = ev.key.keysym.mod & KMOD_SHIFT;

      if (!m_ctrl_pressed && !(ev.key.keysym.mod & KMOD_RSHIFT))
        m_scroll_speed = 32.0f;
    }
    else if (ev.type == SDL_MOUSEMOTION)
    {
      m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);
    }
    else if (ev.type == SDL_MOUSEWHEEL && !m_toolbox_widget->has_mouse_focus() && !m_layers_widget->has_mouse_focus())
    {
      // Scroll or zoom with mouse wheel, if the mouse is not over the toolbox.
      // The toolbox does scrolling independently from the main area.
      if (m_ctrl_pressed)
        m_new_scale = m_sector->get_camera().get_current_scale() + static_cast<float>(ev.wheel.y) * CAMERA_ZOOM_SENSITIVITY;
      else if (m_shift_pressed)
        scroll({ static_cast<float>(ev.wheel.y * -40), static_cast<float>(ev.wheel.x * -40) });
      else
        scroll({ static_cast<float>(ev.wheel.x * -40), static_cast<float>(ev.wheel.y * -40) });
    }

    BIND_SECTOR(*m_sector);
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
Editor::toggle_tile_object_mode()
{
  auto& tilebox = m_toolbox_widget->get_tilebox();
  const auto& input_type = tilebox.get_input_type();
  if (input_type == InputType::OBJECT)
  {
	select_last_tilegroup();
	for(const auto& widget : m_widgets)
	{
	  if (auto toolbar_button = dynamic_cast<EditorToolbarButtonWidget*>(widget.get()))
	  {
		toolbar_button->set_visible(toolbar_button->get_visible_in_tile_mode());
	  }
	}
  }
  else
  {
	select_last_objectgroup();
	for(const auto& widget : m_widgets)
	{
	  if (auto toolbar_button = dynamic_cast<EditorToolbarButtonWidget*>(widget.get()))
	  {
		toolbar_button->set_visible(toolbar_button->get_visible_in_object_mode());
	  }
	}
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
  m_toolbox_widget->select_tilegroup(id);
}

void
Editor::select_last_tilegroup()
{
  m_toolbox_widget->select_last_tilegroup();
}

const std::vector<Tilegroup>&
Editor::get_tilegroups() const
{
  return m_tileset->get_tilegroups();
}

void
Editor::change_tileset()
{
  m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  m_toolbox_widget->get_tilebox().set_input_type(EditorTilebox::InputType::NONE);
  for (const auto& sector : m_level->m_sectors) {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>()) {
      tilemap.set_tileset(m_tileset);
    }
  }
}

void
Editor::select_objectgroup(int id)
{
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
Editor::check_save_prerequisites(const std::function<void ()>& callback) const
{
  if (m_level->is_worldmap())
  {
    callback();
    return;
  }

  bool sector_valid = false, spawnpoint_valid = false;
  for (const auto& sector : m_level->m_sectors)
  {
    if (sector->get_name() == DEFAULT_SECTOR_NAME)
    {
      sector_valid = true;
      for (const auto& spawnpoint : sector->get_objects_by_type<SpawnPointMarker>())
      {
        if (spawnpoint.get_name() == DEFAULT_SPAWNPOINT_NAME)
        {
          spawnpoint_valid = true;
        }
      }
    }
  }

  if(sector_valid && spawnpoint_valid)
  {
    callback();
    return;
  }

  if (!sector_valid)
  {
    /*
    l10n: When translating this message, please keep "main" untranslated (the game expects the name of the sector to be "main").
    */
    Dialog::show_message(_("Couldn't find a sector with the name \"main\".\nPlease change the name of the sector where\nyou'd like the player to start to \"main\""));
  }
  else if (!spawnpoint_valid)
  {
    /*
    l10n: When translating this message, please keep "main" untranslated (the game expects the name of the spawnpoint to be "main").
    */
    Dialog::show_message(_("Couldn't find a spawnpoint with the name \"main\".\nPlease change the name of the spawnpoint where\nyou'd like the player to start to \"main\""));
  }
}

void
Editor::retoggle_undo_tracking()
{
  if (g_config->editor_undo_tracking && !m_undo_widget)
  {
    // Add undo/redo button widgets.
    auto undo_button_widget = std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/undo.png",
        Vector(0, 0), [this]{ undo(); }, Sizef(32.f, 32.f));
    undo_button_widget->set_help_text(_("Undo"));
    auto redo_button_widget = std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/redo.png",
        Vector(32, 0), [this]{ redo(); }, Sizef(32.f, 32.f));
    redo_button_widget->set_help_text(_("Redo"));

    m_undo_widget = undo_button_widget.get();
    m_redo_widget = redo_button_widget.get();

    m_widgets.insert(m_widgets.begin(), std::move(undo_button_widget));
    m_widgets.insert(m_widgets.begin() + 1, std::move(redo_button_widget));
  }
  else if (!g_config->editor_undo_tracking && m_undo_widget)
  {
    // Remove undo/redo button widgets.
    m_widgets.erase(std::remove_if(
                      m_widgets.begin(), m_widgets.end(),
                      [this](const std::unique_ptr<Widget>& widget) {
                          const Widget* ptr = widget.get();
                          return ptr == m_undo_widget || ptr == m_redo_widget;
                      }), m_widgets.end());
    m_undo_widget = nullptr;
    m_redo_widget = nullptr;
  }

  // Toggle undo tracking for all sectors.
  for (const auto& sector : m_level->m_sectors)
    sector->toggle_undo_tracking(g_config->editor_undo_tracking);
}

void
Editor::undo_stack_cleanup()
{
  // Set the undo stack size and perform undo stack cleanup on all sectors.
  for (const auto& sector : m_level->m_sectors)
  {
    sector->set_undo_stack_size(g_config->editor_undo_stack_size);
    sector->undo_stack_cleanup();
  }
}

void
Editor::undo()
{
  BIND_SECTOR(*m_sector);
  m_sector->undo();
  m_layers_widget->update_current_tip();
}

void
Editor::redo()
{
  BIND_SECTOR(*m_sector);
  m_sector->redo();
  m_layers_widget->update_current_tip();
}

IntegrationStatus
Editor::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back("In Editor");
  if (!g_config->hide_editor_levelnames && m_level)
  {
    std::string level_type = (m_level->is_worldmap() ? "worldmap" : "level");
    std::string status_text = "Editing " + level_type + ": " + m_level->get_name();
    
    status.m_details.push_back(status_text);
  }
  return status;
}

void
Editor::pack_addon()
{
  auto id = FileSystem::basename(get_world()->get_basedir());
  auto output_file_path = FileSystem::join(PHYSFS_getWriteDir(), "addons/" + id + ".zip");

  int version = 0;
  if (PHYSFS_exists(output_file_path.c_str()))
  {
    try
    {
      Partio::ZipFileReader zipold(output_file_path);
      auto info_file = zipold.Get_File(id + ".nfo");
      if (info_file)
      {
        auto info_stream = ReaderDocument::from_stream(*info_file);
        auto a = info_stream.get_root().get_mapping();
        a.get("version", version);
      }
    }
    catch(const std::exception& e)
    {
      log_warning << e.what() << std::endl;
    }
  }
  version++;

  Partio::ZipFileWriter zip(output_file_path);
  physfsutil::enumerate_files_recurse(get_world()->get_basedir(),
    [&zip](const std::string& full_path)
    {
      auto os = zip.Add_File(full_path);
      *os << std::ifstream(FileSystem::join(PHYSFS_getWriteDir(), full_path)).rdbuf();
      return false;
    });

  std::stringstream ss;
  Writer info(ss);

  info.start_list("supertux-addoninfo");
  {
    info.write("id", id);
    info.write("version", version);
    info.write("type", get_world()->get_type());

    info.write("title", get_world()->get_title());
    info.write("author", get_level()->get_author());
    info.write("license", get_level()->get_license());
  }
  info.end_list("supertux-addoninfo");

  *zip.Add_File(id + ".nfo") << ss.rdbuf();
}

void
Editor::addControl(const std::string& name, std::unique_ptr<InterfaceControl> new_control, const std::string& description)
{
  float height = 35.f;
  for (const auto& control : m_controls) {
    height = std::max(height, control->get_rect().get_bottom() + 5.f);
  }

  auto control_rect = new_control.get()->get_rect();
  Rectf target_rect = Rectf();
  if (control_rect.get_width() == 0.f || control_rect.get_height() == 0.f)
  {
    target_rect = Rectf(100.f, height, 200.f - 1.0f, height + 20.f);
  } 
  else
  {
    target_rect = Rectf(control_rect.get_left(), height,
                        control_rect.get_right(), height + control_rect.get_height());
  }
  new_control.get()->set_rect(target_rect);

  auto dimensions = Rectf(3.f, height, 100.f, height + 20.f);
  new_control.get()->m_label = std::make_unique<InterfaceLabel>(dimensions, std::move(name), std::move(description));
  //new_control.get()->m_on_change = std::function<void()>([this](){ this->push_version(); });
  m_controls.push_back(std::move(new_control));
}
