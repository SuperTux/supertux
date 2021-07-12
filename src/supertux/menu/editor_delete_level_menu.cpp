//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
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

#include "supertux/menu/editor_delete_level_menu.hpp"
#include <physfs.h>
#include "supertux/levelset.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/editor_level_select_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "util/file_system.hpp"
#include "editor/editor.hpp"
#include "gui/menu_item.hpp"
#include "gui/dialog.hpp"
EditorDeleteLevelMenu::EditorDeleteLevelMenu(std::unique_ptr<Levelset>& levelset, EditorLevelSelectMenu* level_select_menu, EditorLevelsetSelectMenu* levelset_select_menu)
{
  m_level_select_menu = level_select_menu;
  m_levelset_select_menu = levelset_select_menu;
  add_label(_("Delete level"));
  for (int i = 0; i < levelset->get_num_levels(); i++)
  {
    std::string filename = levelset->get_level_filename(i);
    std::string fullpath = FileSystem::join(Editor::current()->get_world()->get_basedir(),filename);
    m_level_full_paths.push_back(fullpath);
    add_entry(i, LevelParser::get_level_name(fullpath));
  }
  add_hl();
  add_back(_("Back"));
}
void
EditorDeleteLevelMenu::menu_action(MenuItem& item)
{
  int id = item.get_id();
  // (int) To avoid compilation warning
  if(id >= 0 && id < (int)m_level_full_paths.size())
  {
    if (LevelParser::get_level_name(m_level_full_paths[id]) == Editor::current()->get_level()->m_name)
      Dialog::show_message(_("You cannot delete level that you are editing!"));
    else
    {
      PHYSFS_delete(m_level_full_paths[id].c_str());
      delete_item(id + 1);
      m_level_full_paths.erase(m_level_full_paths.begin() + id);
      m_level_select_menu->reload_menu();
      if (!Editor::current()->is_level_loaded())
        m_levelset_select_menu->reload_menu();
    }
  }
}