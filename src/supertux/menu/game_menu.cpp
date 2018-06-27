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

#include "supertux/menu/game_menu.hpp"

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "gui/dialog.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_session.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_manager.hpp"
#include "util/gettext.hpp"

GameMenu::GameMenu()
{
  Level* level = GameSession::current()->get_current_level();

  add_label(level->name);
  add_hl();
  add_entry(MNID_CONTINUE, _("Continue"));
  add_entry(MNID_RESETLEVEL, _("Restart Level"));
  add_submenu(_("Options"), MenuStorage::INGAME_OPTIONS_MENU);
  add_hl();
  add_entry(MNID_ABORTLEVEL, _("Abort Level"));
}

void
GameMenu::menu_action(MenuItem* item)
{
  switch (item->id)
  {
    case MNID_CONTINUE:
      MenuManager::instance().clear_menu_stack();
      GameSession::current()->toggle_pause();
      break;

    case MNID_RESETLEVEL:
      Dialog::show_confirmation("Are you sure?", [] {
          MenuManager::instance().clear_menu_stack();
          GameSession::current()->toggle_pause();
          GameSession::current()->reset_button = true;
      });
      break;

    case MNID_ABORTLEVEL:
      Dialog::show_confirmation("Do you really want to exit the level?", [] {
          MenuManager::instance().clear_menu_stack();
          GameSession::current()->abort_level();
      });
      break;
  }
}

/* EOF */
