//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "supertux/menu/editor_menu.hpp"

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "editor/editor.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_manager.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

EditorMenu::EditorMenu()
{
  add_label(_("Level Editor"));
  add_hl();
  add_entry(MNID_RETURNTOEDITOR, _("Return to editor"));
  add_entry(MNID_SAVELEVEL, _("Save current level"));
  add_entry(MNID_TESTLEVEL, _("Test the level"));
  add_entry(MNID_LEVELSEL, _("Edit another level"));
  add_entry(MNID_LEVELSETSEL, _("Choose another level subset"));
  add_submenu(_("Level properties"), MenuStorage::EDITOR_LEVEL_MENU);
  add_hl();
  add_entry(MNID_QUITEDITOR, _("Exit level editor"));
}

EditorMenu::~EditorMenu()
{
  Editor::current()->reactivate_request = true;
}

void
EditorMenu::menu_action(MenuItem* item)
{
  switch (item->id)
  {
    case MNID_RETURNTOEDITOR:
      MenuManager::instance().clear_menu_stack();
      break;

    case MNID_SAVELEVEL:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->save_request = true;
      break;

    case MNID_TESTLEVEL:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->test_request = true;
      break;

    case MNID_LEVELSEL:
      MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVEL_SELECT_MENU);
      break;

    case MNID_LEVELSETSEL:
      MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      break;

    case MNID_QUITEDITOR:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->quit_request = true;
      break;

    default:
      break;
  }
}

/* EOF */
