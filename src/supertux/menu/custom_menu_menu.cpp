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
#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"

enum CustomMenuMenuIDs {
  MNID_RESET,
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
  add_entry(MNID_RESET, _("Reset to defaults"));
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
  case MNID_RESET:
    g_config->menubackcolor = ColorScheme::Menu::back_color;
    g_config->menufrontcolor = ColorScheme::Menu::front_color;
    g_config->hlcolor = ColorScheme::Menu::hl_color;
    g_config->editorcolor = ColorScheme::Editor::default_color;
    g_config->menuroundness = 16.f;
    break;

  default:
    break;
  }
}

/* EOF */
