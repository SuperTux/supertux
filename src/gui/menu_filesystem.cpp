//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "gui/menu_filesystem.hpp"

#include <physfs.h>

#include "addon/addon_manager.hpp"
#include "gui/item_action.hpp"
#include "gui/menu_manager.hpp"
#include "physfs/util.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/gettext.hpp"
#include "util/string_util.hpp"

const size_t FileSystemMenu::s_title_max_chars = 30;
const std::vector<std::string> FileSystemMenu::s_image_extensions = { ".jpg", ".png", ".surface" };

FileSystemMenu::FileSystemMenu(std::string* filename, const std::vector<std::string>& extensions,
                               const std::string& basedir, bool path_relative_to_basedir, std::function<void(std::string)> callback) :
  m_filename(filename),
  // when a basedir is given, 'filename' is relative to basedir, so
  // it's useless as a starting point
  m_directory(basedir.empty() ? (filename ? FileSystem::dirname(*filename) : "/") : basedir),
  m_extensions(extensions),
  m_basedir(basedir),
  m_directories(),
  m_files(),
  m_path_relative_to_basedir(path_relative_to_basedir),
  m_callback(std::move(callback))
{
  AddonManager::current()->unmount_old_addons();

  if (!PHYSFS_exists(m_directory.c_str())) {
    m_directory = "/"; //The filename is probably included in an old add-on.
  }

  refresh_items();
}

FileSystemMenu::~FileSystemMenu()
{
  AddonManager::current()->mount_old_addons();
}

void
FileSystemMenu::refresh_items()
{
  m_items.clear();
  m_directories.clear();
  m_files.clear();
  m_directory = FileSystem::normalize(m_directory);

  // Make sure label doesn't get too long.
  std::string title = m_directory;
  const bool title_large = title.size() > s_title_max_chars;
  while (title.size() > s_title_max_chars) title = title.substr(title.size() - s_title_max_chars);
  if (title_large) title = "..." + title;

  add_label(title);
  add_hl();

  int item_id = 0;

  // Do not allow leaving the data directory
  if (m_directory != "/") {
    m_directories.push_back("..");
  }

  char** dir_files = PHYSFS_enumerateFiles(m_directory.c_str());
  if (dir_files)
  {
    for (const char* const* file = dir_files; *file != nullptr; ++file)
    {
      std::string filepath = FileSystem::join(m_directory, *file);
      if (physfsutil::is_directory(filepath))
      {
        m_directories.push_back(*file);
      }
      else
      {
        if (AddonManager::current()->is_from_old_addon(filepath)) {
          continue;
        }

        if (has_right_suffix(*file))
        {
          m_files.push_back(*file);
        }
      }
    }
    PHYSFS_freeList(dir_files);
  }

  for (const auto& item : m_directories)
  {
    add_entry(item_id, "[" + std::string(item) + "]");
    item_id++;
  }

  for (const auto& item : m_files)
  {
    ItemAction& entry = add_entry(item_id, item);
    if (is_image(item))
    {
      entry.set_preview(FileSystem::join(m_directory, item));
    }
    item_id++;
  }

  add_hl();
  add_entry(-2, _("Open Directory"));
  add_hl();
  add_back(_("Cancel"));

  m_active_item = 2;

  // Re-center menu
  on_window_resize();
  align_for_previews(25.f);
}

bool
FileSystemMenu::has_right_suffix(const std::string& file) const
{
  if (m_extensions.empty())
    return true;

  for (const auto& extension : m_extensions)
    if (StringUtil::has_suffix(file, extension))
      return true;

  return false;
}

bool
FileSystemMenu::is_image(const std::string& file) const
{
  for (const auto& extension : s_image_extensions)
    if (StringUtil::has_suffix(file, extension))
      return true;

  return false;
}

void
FileSystemMenu::menu_action(MenuItem& item)
{
  if (item.get_id() >= 0) {
    size_t id = item.get_id();
    if (id < m_directories.size()) {
      m_directory = FileSystem::join(m_directory, m_directories[id]);
      refresh_items();
    } else {
      id -= m_directories.size();
      if (id < m_files.size()) {
        std::string new_filename = FileSystem::join(m_directory, m_files[id]);

        if (!m_basedir.empty() && m_path_relative_to_basedir) {
          new_filename = FileSystem::relpath(new_filename, m_basedir);
        }

        if (m_filename)
          *m_filename = new_filename;

        if (m_callback)
          m_callback(new_filename);

        MenuManager::instance().pop_menu();
      } else {
        log_warning << "Selected invalid file or directory" << std::endl;
      }
    }
  }
  else if (item.get_id() == -2)
  {
    FileSystem::open_path(FileSystem::join(PHYSFS_getRealDir(m_directory.c_str()), m_directory));
  }
}

/* EOF */
