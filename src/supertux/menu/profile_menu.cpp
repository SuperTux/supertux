//  SuperTux
//  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//                2022 Vankata453
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
#include <physfs.h>
#include <sstream>

#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/profile_name_menu.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

ProfileMenu::ProfileMenu() :
  m_profiles(),
  m_profile_names()
{
  refresh();
}

void
ProfileMenu::refresh()
{
  m_profiles = savegames_util::get_savegames();
  m_profile_names.clear();

  rebuild_menu();
}

void
ProfileMenu::rebuild_menu()
{
  clear();
  add_label(_("Select Profile"));
  add_hl();
  if (m_profiles.empty()) add_inactive(_("No profiles found."));
  bool has_selected_profile = false;
  for (std::size_t i = 0; i < m_profiles.size(); ++i)
  {
    int id = m_profiles[i];
    std::string name;
    for (std::size_t y = 0; y < g_config->profiles.size(); ++y)
    {
      if (g_config->profiles[y].id == id)
        name = g_config->profiles[y].name;
    }
    std::string text;
    if (name.empty())
    {
      text = fmt::format(fmt::runtime(_("profile{}")), id);
    }
    else
    {
      text = name;
    }
    m_profile_names.push_back(text);

    if (id == g_config->profile)
    {
      text = fmt::format(fmt::runtime(_("[{}]")), text);
      has_selected_profile = true;
    }
    if (g_config->developer_mode && !name.empty())
      text = fmt::format(fmt::runtime(_("{} (profile{})")), text, id);

    add_entry(id, text);
    if (id == g_config->profile) set_active_item(id);
  }

  if (!has_selected_profile && !m_profiles.empty())
  {
    add_hl();
    add_inactive(_("No profile selected."));
  }

  add_hl();
  add_entry(-1, _("Add"));
  if (has_selected_profile)
  {
    add_entry(-2, _("Rename"));
  }
  else
  {
    add_inactive(_("Rename"));
  }
  add_hl();
  if (has_selected_profile)
  {
    add_entry(-3, _("Reset"));
  }
  else
  {
    add_inactive(_("Reset"));
  }
  if (!m_profiles.empty())
  {
    add_entry(-4, _("Reset all"));
  }
  else
  {
    add_inactive(_("Reset all"));
  }
  if (has_selected_profile)
  {
    add_entry(-5, _("Delete"));
  }
  else
  {
    add_inactive(_("Delete"));
  }
  if (!m_profiles.empty())
  {
    add_entry(-6, _("Delete all"));
  }
  else
  {
    add_inactive(_("Delete all"));
  }

  add_hl();
  add_back(_("Back"));
}

void
ProfileMenu::menu_action(MenuItem& item)
{
  const auto& id = item.get_id();
  if (id > 0)
  {
    if (g_config->profile == id)
    {
      MenuManager::instance().clear_menu_stack();
      return;
    }
    g_config->profile = id;
    rebuild_menu();
  }
  else if (id == -1)
  {
    MenuManager::instance().push_menu(std::make_unique<ProfileNameMenu>(false));
  }
  else if (id == -2)
  {
    MenuManager::instance().push_menu(std::make_unique<ProfileNameMenu>(true,
    g_config->profile, m_profile_names[g_config->profile - 1]));
  }
  else if (id == -3)
  {
    const std::string message = fmt::format(
      fmt::runtime(_("This will reset all game progress on the profile \"{}\".\nAre you sure?")),
      m_profile_names[g_config->profile - 1]);

    Dialog::show_confirmation(message, []() {
      savegames_util::delete_savegames(g_config->profile, true);
    });
  }
  else if (id == -4)
  {
    Dialog::show_confirmation(_("This will reset your game progress on all profiles. Are you sure?"), [this]() {
      for (std::size_t i = 0; i <= m_profiles.size(); i++)
      {
        savegames_util::delete_savegames(m_profiles[i], true);
      }
    });
  }
  else if (id == -5)
  {
    const std::string message = fmt::format(
    fmt::runtime(_("This will delete the profile \"{}\",\nincluding all game progress on it. Are you sure?")),
    m_profile_names[g_config->profile - 1]);

    Dialog::show_confirmation(message, [this]() {
      savegames_util::delete_savegames(g_config->profile);
      g_config->profiles.erase(
        std::remove_if(g_config->profiles.begin(),
          g_config->profiles.end(),
          [](const auto& profile){
            return profile.id == g_config->profile;
          }),
        g_config->profiles.end());
      g_config->profile = 1;
      refresh();
    });
  }
  else if (id == -6)
  {
    Dialog::show_confirmation(_("This will delete all profiles, including all game progress on them.\nAre you sure?"), [this]() {
      for (std::size_t i = 0; i <= m_profiles.size(); i++)
      {
        savegames_util::delete_savegames(m_profiles[i]);
      }
      g_config->profiles.clear();
      g_config->profile = 1;
      refresh();
    });
  }
  else
  {
    log_warning << "Unknown menu item with id \"" << id << "\" pressed." << std::endl;
  }
}

namespace savegames_util {
  std::vector<int> get_savegames()
  {
    std::vector<int> savegames;
    char **rc = PHYSFS_enumerateFiles("profiles");
    char **i;
    for (i = rc; *i != nullptr; i++)
    if (std::string(*i).substr(0, 7) == "profile") savegames.push_back(std::stoi(std::string(*i).substr(7)));
    PHYSFS_freeList(rc);
    std::sort(savegames.begin(), savegames.end());
    return savegames;
  }

  void delete_savegames(int idx, bool reset)
  {
    const auto& profile_path = "profiles/profile" + std::to_string(idx);
    std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    files(PHYSFS_enumerateFiles(profile_path.c_str()),
        PHYSFS_freeList);
    for (const char* const* filename = files.get(); *filename != nullptr; ++filename)
    {
      std::string filepath = FileSystem::join(profile_path.c_str(), *filename);
      PHYSFS_delete(filepath.c_str());
    }
    if (!reset) PHYSFS_delete(profile_path.c_str());
  }
} // namespace savegames_util

/* EOF */
