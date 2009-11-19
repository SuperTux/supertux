//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "addon/addon.hpp"
#include "addon/addon_manager.hpp"
#include "util/gettext.hpp"

namespace {

bool generate_addons_menu_sorter(const Addon* a1, const Addon* a2)
{
  return a1->title < a2->title;
}

} // namespace

AddonMenu::AddonMenu()
{
  refresh();
}

void
AddonMenu::refresh()
{
  clear();

  AddonManager& adm = AddonManager::get_instance();

  // refresh list of addons
  m_addons = adm.get_addons();
  
  // sort list
  std::sort(m_addons.begin(), m_addons.end(), generate_addons_menu_sorter);


  add_label(_("Add-ons"));
  add_hl();

  // FIXME: don't use macro, use AddonManager::online_available() or so
#ifdef HAVE_LIBCURL
  add_entry(0, std::string(_("Check Online")));
#else
  add_inactive(0, std::string(_("Check Online (disabled)")));
#endif

  //add_hl();

  for (unsigned int i = 0; i < m_addons.size(); i++) 
  {
    const Addon& addon = *m_addons[i];
    std::string text = "";
    
    if (!addon.kind.empty())
    {
      text += addon.kind + " ";
    }
    text += std::string("\"") + addon.title + "\"";

    if (!addon.author.empty())
    {
      text += " by \"" + addon.author + "\"";
    }
    add_toggle(ADDON_LIST_START_ID + i, text, addon.loaded);
  }

  add_hl();
  add_back(_("Back"));
}

void
AddonMenu::check_menu()
{
  int index = check();

  if (index == -1) 
  {
    // do nothing
  }
  else if (index == 0) // check if "Check Online" was chosen
  {
    try 
    {
      AddonManager::get_instance().check_online();
      refresh();
      set_active_item(index);
    } 
    catch (std::exception& e)
    {
      log_warning << "Check for available Add-ons failed: " << e.what() << std::endl;
    }
  }
  else
  {
    // if one of the Addons listed was chosen, take appropriate action
    if ((index >= ADDON_LIST_START_ID) && (index < ADDON_LIST_START_ID) + m_addons.size()) 
    {
      Addon& addon = *m_addons[index - ADDON_LIST_START_ID];
      if (!addon.installed) 
      {
        try 
        {
          AddonManager::get_instance().install(&addon);
        } 
        catch (std::exception& e) 
        {
          log_warning << "Installing Add-on failed: " << e.what() << std::endl;
        }
        set_toggled(index, addon.loaded);
      } 
      else if (!addon.loaded) 
      {
        try 
        {
          AddonManager::get_instance().enable(&addon);
        } 
        catch (std::exception& e) 
        {
          log_warning << "Enabling Add-on failed: " << e.what() << std::endl;
        }
        set_toggled(index, addon.loaded);
      } 
      else 
      {
        try 
        {
          AddonManager::get_instance().disable(&addon);
        } 
        catch (std::exception& e) 
        {
          log_warning << "Disabling Add-on failed: " << e.what() << std::endl;
        }
        set_toggled(index, addon.loaded);
      }
    }
  }
}

/* EOF */
