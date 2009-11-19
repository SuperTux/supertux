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

#include "addon/addon.hpp"
#include "util/gettext.hpp"

AddonMenu::AddonMenu(const std::vector<Addon*>& addons)
{
  add_label(_("Add-ons"));
  add_hl();

  // FIXME: don't use macro, use AddonManager::online_available() or so
#ifdef HAVE_LIBCURL
  add_entry(0, std::string(_("Check Online")));
#else
  add_inactive(0, std::string(_("Check Online (disabled)")));
#endif

  //add_hl();

  for (unsigned int i = 0; i < addons.size(); i++) 
  {
    const Addon& addon = *addons[i];
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

/* EOF */
