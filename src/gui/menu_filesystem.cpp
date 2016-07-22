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
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/item_action.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"

FileSystemMenu::FileSystemMenu(std::string* filename_, const std::vector<std::string>& extensions_) :
  filename(filename_),
  directory(),
  extensions(extensions_),
  directories(),
  files()
{
  AddonManager::current()->unmount_old_addons();

  directory = FileSystem::dirname(*filename);
  if (!PHYSFS_exists(directory.c_str())) {
    directory = "/"; //The filename is probably included in an old add-on.
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
  items.clear();
  directories.clear();
  files.clear();
  directory = FileSystem::normalize(directory);

  add_label(directory);
  add_hl();

  int item_id = 0;

  // Do not allow leaving the data directory
  if (directory != "/") {
    directories.push_back("..");
    add_entry(item_id, "[..]");
    item_id++;
  }

  // Load directories
  {
    std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
      dir_files(PHYSFS_enumerateFiles(directory.c_str()),
            PHYSFS_freeList);
    for(const char* const* file = dir_files.get(); *file != 0; ++file)
    {
      std::string dirpath = FileSystem::join(directory, *file);

      PHYSFS_Stat statbuf;
      PHYSFS_stat(dirpath.c_str(), &statbuf);
      if(statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY)
      {
        directories.push_back(*file);
        add_entry(item_id, "[" + std::string(*file) + "]");
        item_id++;
      }
    }
  }

  // Load files
  {
    char** dir_files = PHYSFS_enumerateFiles(directory.c_str());
    if (dir_files)
    {
      for(const char* const* file = dir_files; *file != 0; ++file)
      {
        if (AddonManager::current()->is_from_old_addon(FileSystem::join(directory, *file))) {
          continue;
        }

        if(has_right_suffix(*file))
        {
          files.push_back(*file);
          add_entry(item_id, *file);
          item_id++;
        }
      }
      PHYSFS_freeList(dir_files);
    }
  }

  add_hl();
  add_back(_("Cancel"));

  active_item = 2;

  // Re-center menu
  on_window_resize();
}

bool
FileSystemMenu::has_right_suffix(const std::string& file) const
{
  for (auto& extension : extensions) {
    if(StringUtil::has_suffix(file, extension))
    {
      return true;
    }
  }
  return false;
}

void
FileSystemMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0) {
    size_t id = item->id;
    if (id < directories.size()) {
      directory = FileSystem::join(directory, directories[id]);
      refresh_items();
    } else {
      id -= directories.size();
      if (id < files.size()) {
        *filename = FileSystem::join(directory, files[id]);
        MenuManager::instance().pop_menu();
      } else {
        log_warning << "Selected invalid file or directory" << std::endl;
      }
    }
  }
}

/* EOF */
