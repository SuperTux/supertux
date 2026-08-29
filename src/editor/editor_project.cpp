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

#include "editor/editor_project.hpp"

#include "editor/editor.hpp"
#include "gui/menu_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/notification.hpp"
#include "object/camera.hpp"
#include "object/spawnpoint.hpp"
#include "physfs/util.hpp"
#include "supertux/constants.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/sector.hpp"
#include "supertux/sector_parser.hpp"
#include "supertux/tile_manager.hpp"
#include "util/file_system.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/string_util.hpp"
#include "util/writer.hpp"

#include "zip_manager.hpp"

#include <physfs.h>

EditorProject::EditorProject() :
  m_world(),
  m_level(),
  m_tileset(),
  m_sector(),
  m_temp_level(true),
  m_levelfile(),
  m_autosave_levelfile(),
  m_level_loaded(),
  m_save_temp_level(false),
  m_time_since_last_save(),
  m_post_save(nullptr)
{
}

void
EditorProject::reactivate()
{
  m_level_loaded = true;
  m_level->reactivate();
  m_sector->activate(Vector(0, 0));
}

void
EditorProject::reset()
{
  m_level_loaded = false;
  m_level.reset();
  m_world.reset();
  m_levelfile.clear();
  m_sector = nullptr;
}

void
EditorProject::close()
{
  remove_autosave_file();

  if (m_world && !get_level_file().empty() && g_config->editor_remember_last_level)
  {
    g_config->editor_last_edited_level = FileSystem::join(get_level_directory(), get_level_file());
  }

  reset();
}

void
EditorProject::level_from_nothing()
{
  m_world.reset();

  m_level = std::make_unique<Level>(false);
  m_level->m_name = "";
  m_level->m_license = LEVEL_DEFAULT_LICENSE;
  m_level->m_tileset = "images/tiles.strf";

  auto sector = SectorParser::from_nothing(*m_level);
  sector->set_name(DEFAULT_SECTOR_NAME);
  m_level->add_sector(std::move(sector));

  m_level->initialize();
  m_levelfile = "";
  m_level_loaded = true;
  //m_reload_request = true;
}

std::string
EditorProject::get_level_path() const
{
  auto world = get_world();
  auto level_file = get_level_file();

  if (world == nullptr)
  {
    return level_file;
  }

  return FileSystem::join(world->get_basedir(), level_file);
}

std::unique_ptr<Level>
EditorProject::get_editable_level()
{
  std::unique_ptr<Level> level;
  ReaderMapping::s_translations_enabled = false;
  try
  {
    auto full_path = get_level_path();
    level = LevelParser::from_file(full_path, is_worldmap(), true);
  }
  catch (const std::exception& err)
  {
    // In case the error was caused by the last edited level, say, not
    // existing/being invalid, let's clear it
    g_config->editor_last_edited_level = "";
    log_warning << "Error loading level '" << m_levelfile << "' in editor: " << err.what() << std::endl;
    throw err;
  }
  ReaderMapping::s_translations_enabled = true;

  return level;
}

void
EditorProject::set_level(std::unique_ptr<Level> level, bool reset)
{
  m_temp_level = (level == nullptr);

  if (level != nullptr)
  {
    // Reload level.
    m_level = std::move(level);
  }
  else
  {
    level_from_nothing();
    g_config->editor_last_edited_level = "";
  }

  if (reset)
  {
    m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  }

  m_level_loaded = true;
}

bool
EditorProject::save_level(const std::string& filename, bool switch_file, const std::function<void ()>& post_save)
{
  if (m_temp_level && !m_save_temp_level)
  {
    if (post_save)
      m_post_save = post_save;
    MenuManager::instance().set_menu(MenuStorage::EDITOR_TEMP_SAVE_MENU);
    return false;
  }

  if (m_save_temp_level)
  {
    m_save_temp_level = false;
    m_temp_level = false;
    // Implied
    switch_file = true;
  }

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

  auto notif = std::make_unique<Notification>("save_level_notif", 3.f);
  notif->set_text(_("Level saved!"));
  MenuManager::instance().set_notification(std::move(notif));

  trigger_post_save();
  return true;
}

void
EditorProject::trigger_post_save()
{
  if (m_post_save)
  {
    m_post_save();
    m_post_save = nullptr;
  }
}

void
EditorProject::reload_level()
{
  // Autosave files : Once the level is loaded, make sure
  // to use the regular file.
  m_levelfile = get_levelname_from_autosave(m_levelfile);
  m_autosave_levelfile = FileSystem::join(get_level_directory(),
                                          get_autosave_from_levelname(m_levelfile));
}

void
EditorProject::autosave(float dt_sec)
{
  if (!m_level || m_temp_level)
  {
    m_time_since_last_save = 0.f;
    return;
  }

  // Auto-save (interval).
  m_time_since_last_save += dt_sec;

  float autosave_frequency_sec =
    static_cast<float>(std::max(g_config->editor_autosave_frequency, 1)) * 60.f;

  if (m_time_since_last_save < autosave_frequency_sec)
    return;

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

void
EditorProject::remove_autosave_file()
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

std::string
EditorProject::get_level_directory() const
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
EditorProject::open_level_directory()
{
  if (m_temp_level)
    return;
  m_level->save(FileSystem::join(get_level_directory(), m_levelfile));
  auto path = FileSystem::join(PHYSFS_getWriteDir(), get_level_directory());
  FileSystem::open_path(path);
}

void
EditorProject::set_sector(Sector* sector)
{
  m_sector = sector;
  m_sector->activate(DEFAULT_SPAWNPOINT_NAME);

  { // Initialize badguy sprites and perform other GameObject related tasks.
    BIND_SECTOR(*m_sector);
    for(auto& object : m_sector->get_objects()) {
      object->after_editor_set();
    }
  }
}

void
EditorProject::load_sector(const std::string& name, bool reset)
{
  auto sector_name = name;
  Vector cam_position(0.0f, 0.0f);

  auto previous_sector = get_sector();
  if (previous_sector != nullptr && !reset)
  {
    sector_name = previous_sector->get_name();
    cam_position = previous_sector->get_camera().get_translation();
  }

  auto sector = m_level->get_sector(sector_name);
  if (!sector)
  {
    sector = m_level->get_sector(0);
  }

  sector->set_undo_stack_size(g_config->editor_undo_stack_size);
  sector->toggle_undo_tracking(g_config->editor_undo_tracking);

  set_sector(sector);

  if (sector != nullptr)
  {
    sector->get_camera().set_mode(Camera::Mode::FREE);

    if (!reset)
    {
      sector->get_camera().set_translation(cam_position);
    }
  }
}

bool
EditorProject::test_project(const std::optional<std::pair<std::string, Vector>>& start_pos)
{
    std::unique_ptr<World> owned_world;
    World* current_world = m_world.get();

    if ((m_level && !current_world) || m_levelfile == "")
    {
        GameManager::current()->start_level(m_level.get(), start_pos, true);
        return true;
    }

    std::string backup_filename = get_autosave_from_levelname(m_levelfile);
    std::string directory = get_level_directory();

    // This is jank to get an owned World pointer, GameManager/World
    // could probably need a refactor to handle this better.
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
        GameManager::current()->start_level(*current_world, backup_filename, start_pos, true);
        return true;
    }
    else
    {
        return GameManager::current()->start_worldmap(*current_world, m_autosave_levelfile, start_pos);
    }
}

void
EditorProject::check_save_prerequisites(const std::function<void ()>& callback) const
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


bool
EditorProject::has_unsaved_changes() const
{
  auto level = get_level();
  bool has_unsaved_changes = !g_config->editor_undo_tracking;
  if (!has_unsaved_changes)
  {
    for (const auto& sector : level->m_sectors)
    {
      if (sector->has_object_changes())
      {
        has_unsaved_changes = true;
        break;
      }
    }
  }
  return has_unsaved_changes;
}

void
EditorProject::check_unsaved_changes(const std::function<void ()>& action)
{
  auto editor = Editor::current();
  if (!m_level_loaded || !has_unsaved_changes())
  {
    action();
    return;
  }

  editor->set_enabled(false);
  auto dialog = std::make_unique<Dialog>();
  if (m_temp_level)
    dialog->set_text(_("This level hasn't been saved yet. Do you want to save it instead?"));
  else
    dialog->set_text(g_config->editor_undo_tracking ? _("This level contains unsaved changes, do you want to save?") :
                                                    _("This level may contain unsaved changes, do you want to save?"));
    dialog->add_default_button(_("Yes"), [this, action, editor] {
      check_save_prerequisites([this, action, editor] {
        save_level("", false, action);
        editor->set_enabled(true);
      });
  });
  dialog->add_button(_("No"), [this, action, editor] {
    action();
    editor->set_enabled(true);
  });
  dialog->add_button(_("Cancel"), [editor] {
    editor->set_enabled(true);
  });

  MenuManager::instance().set_dialog(std::move(dialog));
}

void
EditorProject::pack_addon()
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