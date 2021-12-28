//  SuperTux
//  Copyright (C) 2021 Daniel Ward <weluvgoatz@gmail.com>
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

#include "supertux/menu/custom_menu_menu.hpp"

#include "config.h"

#include "gui/item_goto.hpp"
#include "gui/item_stringselect.hpp"
#include "gui/item_toggle.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"

enum CustomMenuMenuIDs {
  MNID_MENUBACKCOLOR,
  MNID_MENUFRONTCOLOR,
  MNID_HLCOLOR,
  MNID_EDITORCOLOR,
  MNID_MENUROUNDNESS
};

CustomMenuMenu::CustomMenuMenu()
{
  add_label(_("Menu Customization"));
  add_hl();
  add_color(_("Menu Back Color"), &g_config->menubackcolor, MNID_MENUBACKCOLOR);
  add_color(_("Menu Front Color"), &g_config->menufrontcolor, MNID_MENUFRONTCOLOR);
  add_color(_("Divider Line Color"), &g_config->hlcolor, MNID_HLCOLOR);
  add_color(_("Editor Color Scheme"), &g_config->editorcolor, MNID_EDITORCOLOR);
  add_floatfield(_("Menu Roundness"), &g_config->menuroundness, MNID_MENUROUNDNESS);
  add_hl();
  add_back(_("Back"));
}

CustomMenuMenu::~CustomMenuMenu()
{
}

void
CustomMenuMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
  case MNID_MENUBACKCOLOR:
    break;
  case MNID_MENUFRONTCOLOR:
    break;
  case MNID_HLCOLOR:
    break;
  case MNID_EDITORCOLOR:
    break;
  case MNID_MENUROUNDNESS:
    break;
  default:
    break;
  }
}

/* EOF */
