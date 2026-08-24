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

#include <physfs.h>

EditorProject::EditorProject() :
  m_world(),
  m_level(),
  m_tileset(),
  m_temp_level(),
  m_levelfile(),
  m_autosave_levelfile(),
  m_save_temp_level(false),
  m_time_since_last_save()
{
}

void
EditorProject::reset()
{
  m_level.reset();
  m_world.reset();
  m_levelfile.clear();
  m_sector = nullptr;
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

void
EditorProject::set_level(std::unique_ptr<Level> level, bool reset = true)
{
  m_temp_level = (level == nullptr);

  if (!m_temp_level)
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
EditorProject::set_sector()
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