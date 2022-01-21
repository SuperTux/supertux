//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#include "supertux/menu/multiplayer_menu.hpp"

#include "gui/item_toggle.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"

MultiplayerMenu::MultiplayerMenu()
{
  add_label(_("Multiplayer"));
  add_hl();

  auto& automanage_item = add_toggle(-1, _("Auto-manage Players"), &g_config->multiplayer_auto_manage_players);
  automanage_item.set_help(_("Automatically add and remove players when controllers are plugged or unplugged"));

  auto& buzz_item = add_toggle(-1, _("Enable Rumbling Controllers"), &g_config->multiplayer_buzz_controllers);
  buzz_item.set_help(_("Enable vibrating the game controllers.") + " " + _("This feature is currently only used in the options menu."));
  // ^ Separated both translation strings so the latter can be removed if it is
  // no longer true, without requiring a new round of translating

  add_submenu(_("Manage Players"), MenuStorage::MULTIPLAYER_PLAYERS_MENU);

  add_hl();
  add_back(_("Back"));
}

/* EOF */
