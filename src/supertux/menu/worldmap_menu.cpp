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

#include "supertux/menu/worldmap_menu.hpp"

#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/options_select_menu.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "util/gettext.hpp"

WorldmapMenu::WorldmapMenu()
{
  add_label(_("Pause"));
  add_hl();
  add_entry(MNID_RETURNWORLDMAP, _("Continue"));
  add_entry(MNID_OPTIONS, _("Options"));
  add_hl();
  add_entry(MNID_QUITWORLDMAP, _("Leave World"));
}

void
WorldmapMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
    case MNID_RETURNWORLDMAP:
      MenuManager::instance().clear_menu_stack();
      break;

    case MNID_OPTIONS:
      MenuManager::instance().push_menu(std::make_unique<OptionsSelectMenu>(false));
      break;

    case MNID_QUITWORLDMAP:
      MenuManager::instance().clear_menu_stack();
      ScreenManager::current()->pop_screen();
      break;
  }
}

/* EOF */
