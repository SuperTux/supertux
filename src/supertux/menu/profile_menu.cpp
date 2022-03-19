//  SuperTux
//  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/profile_menu.hpp"

#include <fmt/format.h>
#include <sstream>

#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

#include <physfs.h>

ProfileMenu::ProfileMenu()
{
  add_label(_("Select Profile"));
  add_hl();
  for (int i = 1; i <= 5; ++i)
  {
    std::ostringstream out;
    if (i == g_config->profile)
    {
      out << fmt::format(fmt::runtime(_("[Profile {}]")), i);
    }
    else
    {
      out << fmt::format(fmt::runtime(_("Profile {}")), i);
    }
    add_entry(i, out.str());
  }
  add_hl();
  add_entry(6, _("Reset profile"));
  add_entry(7, _("Reset all profiles"));

  add_hl();
  add_back(_("Back"));
}

void
ProfileMenu::menu_action(MenuItem& item)
{
  const auto& id = item.get_id();
  if(id <= 5)
  {
    g_config->profile = item.get_id();
  }
  else if(id == 6)
  {
    Dialog::show_confirmation(_("Deleting your profile will reset your game progress. Are you sure?"), [this]() {
      delete_savegames(g_config->profile);
    });
  }
  else if(id == 7)
  {
    Dialog::show_confirmation(_("This will reset your game progress on all profiles. Are you sure?"), [this]() {
      for (int i = 1; i <= 5; i++) {
        delete_savegames(i);
      }
    });
  }
  MenuManager::instance().clear_menu_stack();
}

void
ProfileMenu::delete_savegames(int idx) const
{
  const auto& profile_path = "profile" + std::to_string(idx);
  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    files(PHYSFS_enumerateFiles(profile_path.c_str()),
          PHYSFS_freeList);
  for (const char* const* filename = files.get(); *filename != nullptr; ++filename)
  {
    std::string filepath = FileSystem::join(profile_path.c_str(), *filename);
    PHYSFS_delete(filepath.c_str());
  }
  PHYSFS_delete(profile_path.c_str());
}

/* EOF */
