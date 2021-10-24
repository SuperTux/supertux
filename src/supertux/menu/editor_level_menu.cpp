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

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "supertux/level.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

EditorLevelMenu::EditorLevelMenu() :
  old_tileset(Editor::current()->get_level()->m_tileset)
{
  bool worldmap = Editor::current()->get_level()->is_worldmap();
  auto level = Editor::current()->get_level();

  add_label(worldmap ? _("Worldmap Settings") :_("Level Settings"));
  add_hl();
  add_textfield(_("Name"), &(level->m_name));
  add_textfield(_("Author"), &(level->m_author));
  add_textfield(_("Contact"), &(level->m_contact));
  add_textfield(_("License"), &(level->m_license));
  add_textfield(_("Level Note"), &(level->m_note));
  add_file(_("Tileset"), &(level->m_tileset), std::vector<std::string>(1, ".strf"), {});

  if (!worldmap) {
    add_floatfield(_("Target Time"), &(level->m_target_time));
  }

  add_hl();
  add_back(_("OK"));
}

EditorLevelMenu::~EditorLevelMenu()
{
  auto editor = Editor::current();
  if (editor == nullptr) {
    return;
  }
  if (editor->get_level()->m_tileset != old_tileset) {
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
EditorLevelMenu::menu_action(MenuItem& item)
{
}

bool
EditorLevelMenu::on_back_action()
{
  auto level = Editor::current()->get_level();
  if (!level->m_name.empty() && !level->m_author.empty() && !level->m_license.empty())
  {
    return true;
  }

  if (level->m_name.empty())
  {
    Dialog::show_message(_("Please enter a name for this level."));
  }
  else if (level->m_author.empty())
  {
    Dialog::show_message(_("Please enter a level author for this level."));
  }
  else if (level->m_license.empty())
  {
    Dialog::show_message(_("Please enter a license for this level."));
  }
  return false;
}

/* EOF */
