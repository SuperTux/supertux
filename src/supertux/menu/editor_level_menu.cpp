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

#include "gui/dialog.hpp"
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

EditorLevelMenu::EditorLevelMenu() :
  old_tileset(Editor::current()->get_level()->tileset)
{
  bool worldmap = Editor::current()->get_worldmap_mode();
  auto level = Editor::current()->get_level();

  add_label(worldmap ? _("Worldmap properties") :_("Level properties"));
  add_hl();
  add_textfield(_("Name"), &(level->name));
  add_textfield(_("Author"), &(level->author));
  add_textfield(_("Contact"), &(level->contact));
  add_textfield(_("License"), &(level->license));
  add_file(_("Tileset"), &(level->tileset), std::vector<std::string>(1, ".strf"));

  if (!worldmap) {
    add_numfield(_("Target time"), &(level->target_time));
  }

  add_hl();
  add_back(_("OK"));
}

EditorLevelMenu::~EditorLevelMenu()
{
  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  if (editor->get_level()->tileset != old_tileset) {
    try
    {
      editor->change_tileset();
    }
    catch(std::exception& e)
    {
      // Lisp Type error might occur.
      log_warning << e.what() << std::endl;
    }
  }
}

void
EditorLevelMenu::menu_action(MenuItem* item)
{
}

bool
EditorLevelMenu::on_back_action()
{
  auto level = Editor::current()->get_level();
  if(!level->name.empty() && !level->author.empty() && !level->license.empty())
  {
    return true;
  }

  if(level->name.empty())
  {
    Dialog::show_message(_("Please enter a name for this level."));
  }
  else if(level->author.empty())
  {
    Dialog::show_message(_("Please enter a level author for this level."));
  }
  else if(level->license.empty())
  {
    Dialog::show_message(_("Please enter a license for this level."));
  }
  return false;
}

/* EOF */
