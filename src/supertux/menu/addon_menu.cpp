//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/addon_menu.hpp"

#include <config.h>
#include <algorithm>
#include <boost/format.hpp>

#include "addon/addon.hpp"
#include "addon/addon_manager.hpp"
#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "util/gettext.hpp"

AddonMenu::AddonMenu() :
  m_addon_manager(*AddonManager::current())
{
  refresh();
}

void
AddonMenu::refresh()
{
  clear();

  // refresh list of addons
  const auto& addons_ref = m_addon_manager.get_addons();
  std::vector<std::reference_wrapper<Addon> > addons;
  std::transform(addons_ref.begin(), addons_ref.end(), std::back_inserter(addons),
                 [](const std::unique_ptr<Addon>& addon) -> Addon& {
                   return *addon.get();
                 });

  // sort list
  std::sort(addons.begin(), addons.end(),
            [](const Addon& lhs, const Addon& rhs)
            {
              return lhs.title < lhs.title;
            });

  add_label(_("Add-ons"));
  add_hl();

  if (!m_addon_manager.has_online_support())
  {
    add_inactive(MNID_CHECK_ONLINE, std::string(_("Check Online (disabled)")));
  }
  else
  {
    add_entry(MNID_CHECK_ONLINE, std::string(_("Check Online")));
  }

  //add_hl();

  for (auto& addon_ : addons)
  {
    Addon& addon = addon_.get();
    std::string text = "";

    if (!addon.kind.empty())
    {
      std::string kind = addon.kind;
      if(addon.kind == "Levelset") {
        kind = _("Levelset");
      }
      else if(addon.kind == "Worldmap") {
        kind = _("Worldmap");
      }
      else if(addon.kind == "World") {
        kind = _("World");
      }
      else if(addon.kind == "Level") {
        kind = _("Level");
      }

      if(!addon.author.empty())
      {
        text = str(boost::format(_("%s \"%s\" by \"%s\""))
                   % kind % addon.title % addon.author);
      }
      else
      {
        // Only addon type and name, no need for translation.
        text = str(boost::format("%s \"%s\"")
                   % kind % addon.title);
      }
    }
    else
    {
      if (!addon.author.empty())
      {
        text = str(boost::format(_("\"%s\" by \"%s\""))
                   % addon.title % addon.author);
      }
      else {
        // Only addon name, no need for translation.
        text = str(boost::format("\"%s\"")
                   % addon.title);
      }
    }
    add_toggle(MNID_ADDON_LIST_START + addon.id, text, addon.loaded);
  }

  add_hl();
  add_back(_("Back"));
}

void
AddonMenu::menu_action(MenuItem* item)
{
  if (item->id == MNID_CHECK_ONLINE) // check if "Check Online" was chosen
  {
    try
    {
      m_addon_manager.check_online();
      refresh();
      set_active_item(item->id);
    }
    catch (std::exception& e)
    {
      log_warning << "Check for available Add-ons failed: " << e.what() << std::endl;
    }
  }
  else if ((MNID_ADDON_LIST_START <= item->id) && (item->id < MNID_ADDON_LIST_START + m_addon_manager.get_num_addons()))
  {
    int addon_id = item->id - MNID_ADDON_LIST_START;
    Addon& addon = m_addon_manager.get_addon(addon_id);
    if (!addon.installed)
    {
      try
      {
        m_addon_manager.install(addon);
      }
      catch (std::exception& e)
      {
        log_warning << "Installing Add-on failed: " << e.what() << std::endl;
      }
      set_toggled(item->id, addon.loaded);
    }
    else if (!addon.loaded)
    {
      try
      {
        m_addon_manager.enable(addon);
      }
      catch (std::exception& e)
      {
        log_warning << "Enabling Add-on failed: " << e.what() << std::endl;
      }
      set_toggled(item->id, addon.loaded);
    }
    else
    {
      try
      {
        m_addon_manager.disable(addon);
      }
      catch (std::exception& e)
      {
        log_warning << "Disabling Add-on failed: " << e.what() << std::endl;
      }
      set_toggled(item->id, addon.loaded);
    }
  }
}

/* EOF */
