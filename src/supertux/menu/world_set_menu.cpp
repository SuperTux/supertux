//  SuperTux
//  Copyright (C) 2015 Matthew <thebatmankiller3@gmail.com>
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

#include "supertux/menu/world_set_menu.hpp"

#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"

WorldSetMenu::WorldSetMenu() :
  WorldPreviewMenu(2.5f, 2)
{
  add_label(_("Start Game"));
  add_hl();
  add_entry(0, _("Story Mode"));
  add_entry(1, _("Contrib Levels"));
  add_hl();
  add_back(_("Back"));

  // Add Story Mode entry.
  const std::string preview_file = "previews/world1.png";
  const std::string basedir = "levels/world1";
  m_world_entries.push_back({ true, "levels/world1", find_preview(preview_file, basedir), { -2, -2 } });
}

void
WorldSetMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
    case 1:
	    MenuManager::instance().push_menu(MenuStorage::CONTRIB_MENU);
	    break;

    default:
      WorldPreviewMenu::menu_action(item);
      break;
  }
}

/* EOF */
