//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "supertux/menu/editor_level_menu.hpp"

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "editor/editor.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/level.hpp"
#include "supertux/screen_manager.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

EditorLevelMenu::EditorLevelMenu()
{
  bool worldmap = Editor::current()->get_worldmap_mode();
  auto level = Editor::current()->get_level();

  add_label(worldmap ? _("Worldmap properties") :_("Level properties"));
  add_hl();
  add_textfield(_("Name"), &(level->name));
  add_textfield(_("Author"), &(level->author));
  add_textfield(_("Contact"), &(level->contact));
  add_textfield(_("License"), &(level->license));

  if (!worldmap) {
    add_script(_("On menukey script"), &(level->on_menukey_script));
    add_numfield(_("Target time"), &(level->target_time));
  }

  add_hl();
  add_back(_("OK"));
}

void
EditorLevelMenu::menu_action(MenuItem* item)
{

}

/* EOF */
