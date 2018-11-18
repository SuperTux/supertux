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

#include <limits>
#include <physfs.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <shellapi.h>
#else
  #include <cstdlib>
#endif

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "editor/layer_icon.hpp"
#include "editor/object_info.hpp"
#include "editor/tile_selection.hpp"
#include "editor/tip.hpp"
#include "editor/tool_icon.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "gui/mousecursor.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/spawnpoint.hpp"
#include "object/tilemap.hpp"
#include "physfs/physfs_file_system.cpp"
#include "supertux/game_manager.hpp"
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
#include "util/reader_mapping.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

bool
Editor::is_active()
{
  auto self = Editor::current();
  return self && self->m_levelloaded && !self->m_leveltested;
}

Editor::Editor() :
  m_level(),
  m_world(),
  m_levelfile(),
  m_test_levelfile(),
  m_worldmap_mode(false),
  m_quit_request(false),
  m_newlevel_request(false),
  m_reload_request(false),
  m_reactivate_request(false),
  m_deactivate_request(false),
  m_save_request(false),
  m_test_request(false),
  m_savegame(),
  m_sector(),
  m_levelloaded(false),
  m_leveltested(false),
  m_tileset(nullptr),
  m_inputcenter(*this),
  m_tileselect(*this),
  m_layerselect(*this),
  m_scroller(*this),
  m_enabled(false),
  m_bgr_surface(Surface::from_file("images/background/forest1.jpg"))
{
}

void
Editor::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  if (m_levelloaded) {
    m_sector->draw(context);
    context.color().draw_filled_rect(Rectf(Vector(0, 0), Vector(static_cast<float>(context.get_width()),
                                                                static_cast<float>(context.get_height()))),
                                     Color(0.0f, 0.0f, 0.0f),
                                     0.0f, std::numeric_limits<int>::min());
  } else {
    context.color().draw_surface_scaled(m_bgr_surface,
                                        Rectf(Vector(0, 0), Vector(static_cast<float>(context.get_width()),
                                                                   static_cast<float>(context.get_height()))),
                                        -100);
  }
  m_inputcenter.draw(context);
  m_tileselect.draw(context);
  m_layerselect.draw(context);
  m_scroller.draw(context);
  MouseCursor::current()->draw(context);
}

void
Editor::update(float dt_sec, const Controller& controller)
{
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
    save_level();
    m_enabled = true;
    m_save_request = false;
  }

  if (m_test_request) {
    m_test_request = false;
    MouseCursor::current()->set_icon(nullptr);
    test_level();
    return;
  }

  if (m_deactivate_request) {
    m_enabled = false;
    m_deactivate_request = false;
    return;
  }

  // update other stuff
  if (is_active()) {
    m_sector->update(0);
    m_tileselect.update(dt_sec);
    m_layerselect.update(dt_sec);
    m_inputcenter.update(dt_sec);
    m_scroller.update(dt_sec);
    update_keyboard(controller);
  }
}

void
Editor::save_level()
{
  m_level->save(m_world ? FileSystem::join(m_world->get_basedir(), m_levelfile) :
              m_levelfile);
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
Editor::test_level()
{
  Tile::draw_editor_images = false;
  Compositor::s_render_lighting = true;
  auto backup_filename = m_levelfile + "~";
  auto directory = get_level_directory();
  auto current_world = (m_world != nullptr) ? m_world.get() : World::load(directory).get();

  m_test_levelfile = FileSystem::join(directory, backup_filename);
  m_level->save(m_test_levelfile);
  if (!m_worldmap_mode)
  {
    GameManager::current()->start_level(current_world, backup_filename);
  }
  else
  {
    GameManager::current()->start_worldmap(current_world, "", m_test_levelfile);
  }

  m_leveltested = true;
}

void
Editor::open_level_directory()
{
  m_level->save(FileSystem::join(get_level_directory(), m_levelfile));
  auto path = FileSystem::join(PHYSFS_getWriteDir(), get_level_directory());

  #if defined(_WIN32) || defined (_WIN64)
    ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
  #else
    #if defined(__APPLE__)
    const char* cmd = std::string("open \"" + path + "\"").c_str();
    #else
    const char* cmd = std::string("xdg-open \"" + path + "\"").c_str();
    #endif

    int ret = system(cmd);
    if (ret < 0)
    {
      log_fatal << "failed to spawn: " << cmd << std::endl;
    }
    else if (ret > 0)
    {
      log_fatal << "error " << ret << " while executing: " << cmd << std::endl;
    }
  #endif
}

void
Editor::set_world(std::unique_ptr<World> w)
{
  m_world = std::move(w);
}

int
Editor::get_tileselect_select_mode() const
{
  return m_tileselect.m_select_mode->get_mode();
}

int
Editor::get_tileselect_move_mode() const
{
  return m_tileselect.m_move_mode->get_mode();
}

bool
Editor::can_scroll_vert() const
{
  return m_levelloaded && (m_sector->get_height() + 32 > static_cast<float>(SCREEN_HEIGHT));
}

bool
Editor::can_scroll_horz() const
{
  return m_levelloaded && (m_sector->get_width() + 128 > static_cast<float>(SCREEN_WIDTH));
}

void
Editor::scroll_left(float speed)
{
  Camera& camera = m_sector->get_camera();
  if (can_scroll_horz()) {
    if (camera.get_translation().x >= speed*32) {
      camera.move(static_cast<int>(-32 * speed), 0);
    } else {
      //When is the camera less than one tile after the left limit, it puts the camera to the limit.
      camera.move(static_cast<int>(-camera.get_translation().x), 0);
    }
    m_inputcenter.update_pos();
  }
}

void
Editor::scroll_right(float speed)
{
  Camera& camera = m_sector->get_camera();
  if (can_scroll_horz()) {
    if (camera.get_translation().x <= m_sector->get_width() - static_cast<float>(SCREEN_WIDTH) + 128.0f - 32.0f * speed) {
      camera.move(static_cast<int>(32 * speed), 0);
    } else {
      //When is the camera less than one tile after the right limit, it puts the camera to the limit.
      // The limit is shifted 128 pixels to the right due to the input gui.
      camera.move(static_cast<int>(m_sector->get_width() - camera.get_translation().x - static_cast<float>(SCREEN_WIDTH) + 128.0f), 0);
    }
    m_inputcenter.update_pos();
  }
}

void
Editor::scroll_up(float speed)
{
  Camera& camera = m_sector->get_camera();
  if (can_scroll_vert()) {
    if (camera.get_translation().y >= speed*32) {
      camera.move(0, static_cast<int>(-32 * speed));
    } else {
      //When is the camera less than one tile after the top limit, it puts the camera to the limit.
      camera.move(0, static_cast<int>(-camera.get_translation().y));
    }
    m_inputcenter.update_pos();
  }
}

void
Editor::scroll_down(float speed)
{
  Camera& camera = m_sector->get_camera();
  if (can_scroll_vert()) {
    if (camera.get_translation().y <= m_sector->get_height() - static_cast<float>(SCREEN_HEIGHT) - 32.0f * speed) {
      camera.move(0, static_cast<int>(32 * speed));
    } else {
      //When is the camera less than one tile after the bottom limit, it puts the camera to the limit.
      // The limit is shifted 32 pixels to the bottom due to the layer toolbar.
      camera.move(0, static_cast<int>(m_sector->get_height() - camera.get_translation().y - static_cast<float>(SCREEN_HEIGHT) + 32.0f));
    }
    m_inputcenter.update_pos();
  }
}

void
Editor::esc_press()
{
  m_enabled = false;
  m_inputcenter.delete_markers();
  MenuManager::instance().set_menu(MenuStorage::EDITOR_MENU);
}

void
Editor::update_keyboard(const Controller& controller)
{
  if (!m_enabled){
    return;
  }

  if (controller.pressed(Controller::ESCAPE)) {
    esc_press();
    return;
  }

  if (controller.hold(Controller::LEFT)) {
    scroll_left();
  }

  if (controller.hold(Controller::RIGHT)) {
    scroll_right();
  }

  if (controller.hold(Controller::UP)) {
    scroll_up();
  }

  if (controller.hold(Controller::DOWN)) {
    scroll_down();
  }
}

void
Editor::load_layers()
{
  m_layerselect.m_selected_tilemap = nullptr;
  m_layerselect.m_layer_icons.clear();

  bool tsel = false;
  for (auto& i : m_sector->get_objects()) {
    auto go = i.get();
    auto mo = dynamic_cast<MovingObject*>(go);
    if ( !mo && go->is_saveable() ) {
      m_layerselect.add_layer(go);

      auto tm = dynamic_cast<TileMap*>(go);
      if (tm) {
        if ( !tm->is_solid() || tsel ) {
          tm->m_editor_active = false;
        } else {
          m_layerselect.m_selected_tilemap = tm;
          tm->m_editor_active = true;
          tsel = true;
        }
      }
    }
  }

  m_layerselect.sort_layers();
  m_layerselect.refresh_sector_text();
}

void
Editor::load_sector(const std::string& name)
{
  m_sector = m_level->get_sector(name);
  if (!m_sector) {
    size_t i = 0;
    m_sector = m_level->get_sector(i);
  }
  m_sector->activate("main");
  load_layers();
}

void
Editor::load_sector(size_t id)
{
  m_sector = m_level->get_sector(id);
  m_sector->activate("main");
  load_layers();
}

void
Editor::reload_level()
{
  m_reload_request = false;
  m_enabled = true;
  m_tileselect.m_input_type = EditorInputGui::IP_NONE;
  // Re/load level
  m_level = nullptr;
  m_levelloaded = true;

  ReaderMapping::s_translations_enabled = false;
  m_level = LevelParser::from_file(m_world ? FileSystem::join(m_world->get_basedir(),
                                                          m_levelfile) : m_levelfile);
  ReaderMapping::s_translations_enabled = true;

  m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  load_sector("main");
  m_sector->activate("main");
  m_sector->get_camera().set_mode(Camera::MANUAL);
  m_layerselect.refresh_sector_text();
  m_tileselect.update_mouse_icon();
}

void
Editor::quit_editor()
{
  //Quit level editor
  m_world = nullptr;
  m_levelfile = "";
  m_levelloaded = false;
  m_quit_request = false;
  m_enabled = false;
  Tile::draw_editor_images = false;
  ScreenManager::current()->pop_screen();
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
  m_tileselect.setup();
  m_layerselect.setup();
  m_savegame.reset(new Savegame("levels/misc"));
  m_savegame->load();

  // Reactivate the editor after level test
  if (m_leveltested) {
    if (!m_test_levelfile.empty())
    {
      // Try to remove the test level using the PhysFS file system
      if (PhysFSFileSystem::remove(m_test_levelfile) != 0)
      {
        // This file is not inside any PhysFS mounts,
        // try to remove this using normal file system
        // methods.
        FileSystem::remove(m_test_levelfile);
      }
    }
    m_leveltested = false;
    Tile::draw_editor_images = true;
    m_level->reactivate();
    m_sector->activate(m_sector->get_player().get_pos());
    MenuManager::instance().clear_menu_stack();
    SoundManager::current()->stop_music();
    m_deactivate_request = false;
    m_enabled = true;
    m_tileselect.update_mouse_icon();
  }
}

void
Editor::resize()
{
  // Calls on window resize.
  m_tileselect.resize();
  m_layerselect.resize();
  m_inputcenter.update_pos();
}

void
Editor::event(const SDL_Event& ev)
{
  if (m_enabled) {
    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F6) {
      Compositor::s_render_lighting = !Compositor::s_render_lighting;
    }

    BIND_SECTOR(*m_sector);

    if ( m_tileselect.event(ev) ) {
      return;
    }

    if ( m_layerselect.event(ev) ) {
      return;
    }

    if ( m_scroller.event(ev) ) {
      return;
    }
    m_inputcenter.event(ev);
  }
}

void
Editor::update_node_iterators()
{
  m_inputcenter.update_node_iterators();
}

void
Editor::delete_markers()
{
  m_inputcenter.delete_markers();
}

void
Editor::sort_layers()
{
  m_layerselect.sort_layers();
}

void
Editor::select_tilegroup(int id)
{
  m_tileselect.m_active_tilegroup.reset(new Tilegroup(m_tileset->get_tilegroups()[id]));
  m_tileselect.m_input_type = EditorInputGui::IP_TILE;
  m_tileselect.reset_pos();
  m_tileselect.update_mouse_icon();
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
  m_tileselect.m_input_type = EditorInputGui::IP_NONE;
  for (const auto& sector : m_level->m_sectors) {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>()) {
      tilemap.set_tileset(m_tileset);
    }
  }
}

void
Editor::select_objectgroup(int id)
{
  m_tileselect.m_active_objectgroup = id;
  m_tileselect.m_input_type = EditorInputGui::IP_OBJECT;
  m_tileselect.reset_pos();
  m_tileselect.update_mouse_icon();
}

const std::vector<ObjectGroup>&
Editor::get_objectgroups() const
{
  return m_tileselect.m_object_info->m_groups;
}

void
Editor::check_save_prerequisites(bool& sector_valid, bool& spawnpoint_valid) const
{
  if (m_worldmap_mode)
  {
    sector_valid = true;
    spawnpoint_valid = true;
    return;
  }
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
}

/* EOF */
