//  SuperTux
//  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <sstream>
#include "gameconfig.hpp"
#include "gettext.hpp"
#include "gui/menu.hpp"

enum ProfileMenuIDs {
  
};

class ProfileMenu : public Menu
{
public:
  ProfileMenu() {
    add_label(_("Select Profile"));
    add_hl();
    for(int i = 0; i < 5; ++i)
      {
        std::ostringstream out;
        out << "Profile " << i+1;
        add_entry(i+1, out.str());
      }

    add_hl();
    add_back(_("Back"));
  }

  void menu_action(MenuItem* item) {
    config->profile = item->id;
    Menu::set_current(0);
  }
};

Menu* profile_menu = 0;

Menu* get_profile_menu()
{
  //static ProfileMenu menu;
  profile_menu = new ProfileMenu();
  return profile_menu;
}

void free_profile_menu()
{
  delete profile_menu;
  profile_menu = 0;
}

/* EOF */
