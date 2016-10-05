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

#include <util/format.hpp>
#include <sstream>

#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/gettext.hpp"

ProfileMenu::ProfileMenu()
{
  add_label(_("Select Profile"));
  add_hl();
  for(int i = 1; i <= 5; ++i)
  {
    std::ostringstream out;
    if (i == g_config->profile)
    {
      out << format("[Profile %s]", i);
    }
    else
    {
      out << format("Profile %s", i);
    }
    add_entry(i, out.str());
  }

  add_hl();
  add_back(_("Back"));
}

void
ProfileMenu::menu_action(MenuItem* item)
{
  g_config->profile = item->id;
  MenuManager::instance().clear_menu_stack();
}

/* EOF */
