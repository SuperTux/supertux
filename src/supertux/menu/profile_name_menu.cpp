//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#include "supertux/menu/profile_name_menu.hpp"

#include <fmt/format.h>
#include <physfs.h>

#include "gui/dialog.hpp"
#include "gui/item_textfield.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/profile_menu.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

ProfileNameMenu::ProfileNameMenu(bool rename, int id, std::string name) :
  m_rename(rename),
  m_current_profile_id(id),
  m_current_profile_name(name),
  m_profile_name()
{
  add_label(m_rename ? fmt::format(fmt::runtime(_("Rename \"{}\"")), m_current_profile_name) : _("Add profile"));
  add_hl();

  add_textfield(_("Name"), &m_profile_name)
    .set_help(_("Profile names must have a maximum of 20 characters."));

  add_entry(1, m_rename ? _("Rename") : _("Create"));

  add_hl();
  add_back(_("Back"));
}

void
ProfileNameMenu::menu_action(MenuItem& item)
{
  if (item.get_id() <= 0)
    return;

  if (m_profile_name.size() > 20)
  {
    Dialog::show_message(_("Profile names must have a maximum of 20 characters.\nPlease choose a different name."));
    return;
  }

  if (!m_rename)
  {
    //Find the smallest available profile ID.
    const std::vector<int> savegames = savegames_util::get_savegames();
    int id = 1;
    if (!savegames.empty()) id = savegames.back() + 1;

    const std::string profile_path = "profile" + std::to_string(id);
    if (!PHYSFS_mkdir(profile_path.c_str()))
    {
      log_warning << "Error creating folder for profile " << id << std::endl;
      Dialog::show_message(_("An error occured while creating the profile."));
      return;
    }

    //Delete ID from profile config data, in case it exists.
    g_config->profiles.erase(
      std::remove_if(g_config->profiles.begin(),
      g_config->profiles.end(),
      [id](const auto& profile) { 
          return profile.id == id;
      }), g_config->profiles.end());

    if (!m_profile_name.empty()) g_config->profiles.push_back({id, m_profile_name});
    g_config->profile = id;
  }
  else
  {
    //Delete ID from profile config data, in case it exists.
    g_config->profiles.erase(
      std::remove_if(g_config->profiles.begin(),
      g_config->profiles.end(),
      [this](const auto& profile) { 
          return profile.id == m_current_profile_id;
      }), g_config->profiles.end());

    if (!m_profile_name.empty()) g_config->profiles.push_back({m_current_profile_id, m_profile_name});
  }

  MenuManager::instance().pop_menu();
  MenuManager::instance().current_menu()->refresh();
}

/* EOF */
