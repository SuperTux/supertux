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

#include <fstream>
#include <sstream>
#include <limits>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

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
#include "editor/undo_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "math/util.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/spawnpoint.hpp"
#include "object/tilemap.hpp"
#include "physfs/util.hpp"
#include "sdk/integration.hpp"
#include "sprite/sprite_manager.hpp"
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

bool Editor::s_resaving_in_progress = false;

bool
Editor::is_active()
{
  if (s_resaving_in_progress) {
    return true;
  } else {
    auto* self = Editor::current();
    return self && !self->m_leveltested;
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
  m_savegame(),
  m_particle_editor_filename(),
  m_sector(),
  m_levelloaded(false),
  m_leveltested(false),
  m_tileset(nullptr),
  m_widgets(),
  m_overlay_widget(),
  m_toolbox_widget(),
  m_layers_widget(),
  m_enabled(false),
  m_bgr_surface(Surface::from_file("images/engine/menu/bg_editor.png")),
  m_undo_manager(new UndoManager),
  m_ignore_sector_change(false),
  m_level_first_loaded(false),
  m_time_since_last_save(0.f),
  m_scroll_speed(32.0f)
{
  auto toolbox_widget = std::make_unique<EditorToolboxWidget>(*this);
  auto layers_widget = std::make_unique<EditorLayersWidget>(*this);
  auto overlay_widget = std::make_unique<EditorOverlayWidget>(*this);

  m_toolbox_widget = toolbox_widget.get();
  m_layers_widget = layers_widget.get();
  m_overlay_widget = overlay_widget.get();

  auto undo_button_widget = std::make_unique<ButtonWidget>("images/engine/editor/undo.png",
    Vector(10, 10), [this]{ undo(); });
  auto redo_button_widget = std::make_unique<ButtonWidget>("images/engine/editor/redo.png",
    Vector(60, 10), [this]{ redo(); });

  m_widgets.push_back(std::move(undo_button_widget));
  m_widgets.push_back(std::move(redo_button_widget));
  m_widgets.push_back(std::move(toolbox_widget));
  m_widgets.push_back(std::move(layers_widget));
  m_widgets.push_back(std::move(overlay_widget));
}

Editor::~Editor()
{
}

void
Editor::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  if (m_levelloaded) {
  for(const auto& widget : m_widgets) {
    widget->draw(context);
  }

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
  // Auto-save (interval)
  if (m_level) {
    m_time_since_last_save += dt_sec;
    if (m_time_since_last_save >= static_cast<float>(std::max(
        g_config->editor_autosave_frequency, 1)) * 60.f) {
      m_time_since_last_save = 0.f;
      std::string backup_filename = get_autosave_from_levelname(m_levelfile);
      std::string directory = get_level_directory();

      // Set the test level file even though we're not testing, so that
      // if the user quits the editor without ever testing, it'll delete
      // the autosave file anyways
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

  // Pass all requests
  if (m_reload_request) {
    reload_level();
  }

  if (m_quit_request) {
    quit_editor();
  }

  if (m_newlevel_request) {
    //Create new level
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
    ScreenManager::current()->push_screen(move(screen));
    return;
  }

  if (m_deactivate_request) {
    m_enabled = false;
    m_deactivate_request = false;
    return;
  }

  // update other stuff
  if (m_levelloaded && !m_leveltested) {
    BIND_SECTOR(*m_sector);

    for (auto& object : m_sector->get_objects()) {
      object->editor_update();
    }

    for (const auto& widget : m_widgets) {
      widget->update(dt_sec);
    }

    // Now that all widgets have been updated, which should have relinquished
    // pointers to objects marked for deletion, we can actually delete them.
    m_sector->flush_game_objects();

    update_keyboard(controller);
  }
}

void
Editor::remove_autosave_file()
{
  // Clear the auto-save file
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

void
Editor::save_level(const std::string& filename, bool switch_file)
{
  auto file = !filename.empty() ? filename : m_levelfile;

  if (switch_file)
    m_levelfile = filename;

  m_undo_manager->reset_index();
  m_level->save(m_world ? FileSystem::join(m_world->get_basedir(), file) : file);
  m_time_since_last_save = 0.f;
  remove_autosave_file();
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
  return std::string(basedir);
}

void
Editor::test_level(const boost::optional<std::pair<std::string, Vector>>& test_pos)
{
  m_overlay_widget->reset_action_press();

  Tile::draw_editor_images = false;
  Compositor::s_render_lighting = true;
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
    GameManager::current()->start_level(*current_world, backup_filename, test_pos);
  }
  else
  {
    GameManager::current()->start_worldmap(*current_world, "", m_autosave_levelfile);
  }

  m_leveltested = true;
}

void
Editor::open_level_directory()
{
  m_level->save(FileSystem::join(get_level_directory(), m_levelfile));
  auto path = FileSystem::join(PHYSFS_getWriteDir(), get_level_directory());
  FileSystem::open_path(path);
}

void
Editor::set_world(std::unique_ptr<World> w)
{
  m_world = std::move(w);
}

int
Editor::get_tileselect_select_mode() const
{
  return m_toolbox_widget->get_tileselect_select_mode();
}

int
Editor::get_tileselect_move_mode() const
{
  return m_toolbox_widget->get_tileselect_move_mode();
}

void
Editor::scroll(const Vector& velocity)
{
  if (!m_levelloaded) return;

  Rectf bounds(0.0f,
               0.0f,
               std::max(0.0f, m_sector->get_width() - static_cast<float>(SCREEN_WIDTH - 128)),
               std::max(0.0f, m_sector->get_height() - static_cast<float>(SCREEN_HEIGHT - 32)));
  Camera& camera = m_sector->get_camera();
  Vector pos = camera.get_translation() + velocity;
  pos = Vector(math::clamp(pos.x, bounds.get_left(), bounds.get_right()),
               math::clamp(pos.y, bounds.get_top(), bounds.get_bottom()));
  camera.set_translation(pos);

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
  if (!m_enabled){
    return;
  }

  
  if (!MenuManager::instance().has_dialog())
  {
    if (controller.pressed(Control::ESCAPE)) {
      esc_press();
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
}

void
Editor::load_sector(const std::string& name)
{
  Sector* sector = m_level->get_sector(name);
  if (!sector) {
    sector = m_level->get_sector(0);
  }
  set_sector(sector);
}

void
Editor::set_sector(Sector* sector)
{
  if (!sector) return;

  m_sector = sector;
  m_sector->activate("main");

  { // initialize badguy sprites and other GameObject stuff
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
    log_fatal << "deleting the last sector is not allowed" << std::endl;
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
  std::string sector_name = "main";
  Vector translation(0.0f, 0.0f);

  if (!reset && m_sector) {
    translation = m_sector->get_camera().get_translation();
    sector_name = m_sector->get_name();
  }

  m_reload_request = false;
  m_enabled = true;

  if (reset) {
    m_toolbox_widget->set_input_type(EditorToolboxWidget::InputType::NONE);
  }

  // Re/load level
  m_level = nullptr;
  m_levelloaded = true;

  m_level = std::move(level);

  if (reset) {
    m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  }

  load_sector(sector_name);

  if (m_sector != nullptr)
  {
    m_sector->activate(sector_name);
    m_sector->get_camera().set_mode(Camera::Mode::MANUAL);

    if (!reset) {
      m_sector->get_camera().set_translation(translation);
    }
  }

  m_layers_widget->refresh_sector_text();
  m_toolbox_widget->update_mouse_icon();
  m_overlay_widget->on_level_change();
  
  if (!m_level_first_loaded)
  {
    m_undo_manager->try_snapshot(*m_level);
    m_undo_manager->reset_index();
    m_level_first_loaded = true;
  }
}

void
Editor::reload_level()
{
  ReaderMapping::s_translations_enabled = false;
  set_level(LevelParser::from_file(m_world ?
                                   FileSystem::join(m_world->get_basedir(), m_levelfile) : m_levelfile,
                                   StringUtil::has_suffix(m_levelfile, ".stwm"),
                                   true));
  ReaderMapping::s_translations_enabled = true;

  // Autosave files : Once the level is loaded, make sure
  // to use the regular file
  m_levelfile = get_levelname_from_autosave(m_levelfile);
  m_autosave_levelfile = FileSystem::join(get_level_directory(),
                                          get_autosave_from_levelname(m_levelfile));
}

void
Editor::quit_editor()
{
  m_quit_request = false;

  auto quit = [this] ()
  {
    remove_autosave_file();

    //Quit level editor
    m_world = nullptr;
    m_levelfile = "";
    m_levelloaded = false;
    m_enabled = false;
    Tile::draw_editor_images = false;
    ScreenManager::current()->pop_screen();
#ifdef __EMSCRIPTEN__
    int persistent = EM_ASM_INT({
      return supertux2_ispersistent();
    }, 0); // EM_ASM_INT is a variadic macro and Clang requires at least 1 value for the variadic argument
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
  if (m_undo_manager->has_unsaved_changes() && m_levelloaded)
  {
    m_enabled = false;
    auto dialog = std::make_unique<Dialog>();
    dialog->set_text(_("This level contains unsaved changes, do you want to save?"));
    dialog->add_default_button(_("Yes"), [this, action] {
      check_save_prerequisites([this, action] {
        save_level();
        action();
        m_enabled = true;
      });
    });
    dialog->add_button(_("No"), [this, action] {
      action();
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
Editor::leave()
{
  MouseCursor::current()->set_icon(nullptr);
  Compositor::s_render_lighting = true;
}

void
Editor::setup()
{
  Tile::draw_editor_images = true;
  Sector::s_draw_solids_only = false;
  if (!m_levelloaded) {

#if 0
    if (AddonManager::current()->is_old_addon_enabled()) {
      auto dialog = std::make_unique<Dialog>();
      dialog->set_text(_("Some obsolete add-ons are still active\nand might cause collisions with default Super Tux structure.\nYou can still enable these add-ons in the menu.\nDisabling these add-ons will not delete your game progress."));
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
  }
  m_toolbox_widget->setup();
  m_layers_widget->setup();
  m_savegame = Savegame::from_file("levels/misc");

  // Reactivate the editor after level test
  if (m_leveltested) {
    m_leveltested = false;
    Tile::draw_editor_images = true;
    m_level->reactivate();
    m_sector->activate(m_sector->get_player().get_pos());
    MenuManager::instance().clear_menu_stack();
    SoundManager::current()->stop_music();
    m_deactivate_request = false;
    m_enabled = true;
    m_toolbox_widget->update_mouse_icon();
  }
  
}

void
Editor::resize()
{
  // Calls on window resize.
  m_toolbox_widget->resize();
  m_layers_widget->resize();
  m_overlay_widget->update_pos();
}

void
Editor::event(const SDL_Event& ev)
{
  if (!m_enabled) return;

  try
  {
	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_t &&
        ev.key.keysym.mod & KMOD_CTRL) {
		test_level(boost::none);
		}

	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_s &&
        ev.key.keysym.mod & KMOD_CTRL) {
		save_level();
		}

	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_z &&
        ev.key.keysym.mod & KMOD_CTRL) {
		undo();
		}

	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_y &&
        ev.key.keysym.mod & KMOD_CTRL) {
		redo();
		}

  if (ev.type == SDL_KEYDOWN)
  {
    if (ev.key.keysym.mod & KMOD_SHIFT)
    {
      m_scroll_speed = 96.0f;
    }
    else if (ev.key.keysym.mod & KMOD_CTRL)
    {
      m_scroll_speed = 16.0f;
    }
    else
    {
      m_scroll_speed = 32.0f;
    }
  }

  

    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F6) {
      Compositor::s_render_lighting = !Compositor::s_render_lighting;
      return;
    }

    m_ignore_sector_change = false;

    BIND_SECTOR(*m_sector);

    for(const auto& widget : m_widgets) {
      if (widget->event(ev))
        break;
    }

    // unreliable heuristic to snapshot the current state for future undo
    if (((ev.type == SDL_KEYUP && ev.key.repeat == 0 &&
         ev.key.keysym.sym != SDLK_LSHIFT &&
         ev.key.keysym.sym != SDLK_RSHIFT &&
         ev.key.keysym.sym != SDLK_LCTRL &&
         ev.key.keysym.sym != SDLK_RCTRL) ||
         ev.type == SDL_MOUSEBUTTONUP))
    {
      if (!m_ignore_sector_change) {
        if (m_level) {
          m_undo_manager->try_snapshot(*m_level);
        }
      }
    }

    // Scroll with mouse wheel, if the mouse is not over the toolbox.
    // The toolbox does scrolling independently from the main area.
    if (ev.type == SDL_MOUSEWHEEL && !m_toolbox_widget->has_mouse_focus() && !m_layers_widget->has_mouse_focus()) {
      float scroll_x = static_cast<float>(ev.wheel.x * -32);
      float scroll_y = static_cast<float>(ev.wheel.y * -32);
      scroll({scroll_x, scroll_y});
    }
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
  m_toolbox_widget->select_tilegroup(id);
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
  m_toolbox_widget->set_input_type(EditorToolboxWidget::InputType::NONE);
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

const std::vector<ObjectGroup>&
Editor::get_objectgroups() const
{
  return m_toolbox_widget->get_object_info().m_groups;
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
    if (sector->get_name() == "main")
    {
      sector_valid = true;
      for (const auto& spawnpoint : sector->get_objects_by_type<SpawnPointMarker>())
      {
        if (spawnpoint.get_name() == "main")
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
  else
  {
    if (!sector_valid)
    {
      Dialog::show_message(_("Couldn't find a \"main\" sector.\nPlease change the name of the sector where\nyou'd like the player to start to \"main\""));
    }
    else if (!spawnpoint_valid)
    {
      Dialog::show_message(_("Couldn't find a \"main\" spawnpoint.\n Please change the name of the spawnpoint where\nyou'd like the player to start to \"main\""));
    }
  }

}

void
Editor::undo()
{
  log_info << "attempting undo" << std::endl;
  auto level = m_undo_manager->undo();
  if (level) {
    set_level(std::move(level), false);
    m_ignore_sector_change = true;
  } else {
    log_info << "undo failed" << std::endl;
  }
}

void
Editor::redo()
{
  log_info << "attempting redo" << std::endl;
  auto level = m_undo_manager->redo();
  if (level) {
    set_level(std::move(level), false);
    m_ignore_sector_change = true;
  } else {
    log_info << "redo failed" << std::endl;
  }
}

IntegrationStatus
Editor::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back("In Editor");
  if (!g_config->hide_editor_levelnames && m_level)
  {
    if (m_level->is_worldmap())
    {
      status.m_details.push_back("Editing worldmap: " + m_level->get_name());
    }
    else
    {
      status.m_details.push_back("Editing level: " + m_level->get_name());
    }
  }
  return status;
}

PHYSFS_EnumerateCallbackResult
Editor::foreach_recurse(void *data, const char *origdir, const char *fname)
{
  auto full_path = FileSystem::join(std::string(origdir), std::string(fname));

  PHYSFS_Stat ps;
  PHYSFS_stat(full_path.c_str(), &ps);
  if (ps.filetype == PHYSFS_FILETYPE_DIRECTORY)
  {
    PHYSFS_enumerate(full_path.c_str(), foreach_recurse, data);
  }
  else
  {
    auto* zip = static_cast<Partio::ZipFileWriter*>(data);
    auto os = zip->Add_File(full_path);
    auto filename = FileSystem::join(std::string(PHYSFS_getWriteDir()), full_path);
    *os << std::ifstream(filename).rdbuf();
  }

  return PHYSFS_ENUM_OK;
}

void
Editor::pack_addon()
{
  auto id = FileSystem::basename(get_world()->get_basedir());

  int version = 0;
  try
  {
    Partio::ZipFileReader zipold(FileSystem::join(std::string(PHYSFS_getWriteDir()), "addons/" + id + ".zip"));
    auto info_file = zipold.Get_File(id + ".nfo");
    if (info_file)
    {
      auto info_stream = ReaderDocument::from_stream(*info_file);
      boost::optional<ReaderMapping> rm;
      auto a = info_stream.get_root().get_mapping();
      a.get("version", version);
    }
  }
  catch(const std::exception& e)
  {
    log_warning << e.what() << std::endl;
  }
  version++;

  Partio::ZipFileWriter zip(FileSystem::join(std::string(PHYSFS_getWriteDir()), "addons/" + id + ".zip"));
  PHYSFS_enumerate(get_world()->get_basedir().c_str(), foreach_recurse, &zip);

  std::stringstream ss;
  Writer info(ss);

  info.start_list("supertux-addoninfo");
  {
    info.write("id", id);
    info.write("version", version);

    if (get_world()->is_levelset())
      info.write("type", "levelset");
    else if (get_world()->is_worldmap())
      info.write("type", "worldmap");

    info.write("title", get_world()->get_title());
    info.write("author", get_level()->get_author());
    info.write("license", get_level()->get_license());
  }
  info.end_list("supertux-addoninfo");

  *zip.Add_File(id + ".nfo") << ss.rdbuf();
}

/* EOF */
