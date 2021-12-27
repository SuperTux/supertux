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

#include "supertux/menu/editor_delete_levelset_menu.hpp"

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "physfs/util.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

EditorDeleteLevelsetMenu::EditorDeleteLevelsetMenu(EditorLevelsetSelectMenu* editor_levelset_select_menu) :
  m_editor_levelset_select_menu(editor_levelset_select_menu)
{
  refresh();
}

void
EditorDeleteLevelsetMenu::refresh()
{
  clear();
  add_label(_("Delete World"));
  add_hl();
  unsigned int i = 0;
  for(std::string& level_world : m_editor_levelset_select_menu->m_contrib_worlds)
  {
    try
    {
      std::unique_ptr<World> world = World::from_directory(level_world);
      if (world->hide_from_contribs())
      {
        continue;
      }
      if (!world->is_levelset() && !world->is_worldmap())
      {
        log_warning << level_world << ": unknown World type" << std::endl;
        continue;
      }
      auto title = world->get_title();
      if (title.empty())
      {
        continue;
      }
      add_entry(i++, title);
    }
    catch(std::exception& e)
    {
      log_info << "Couldn't parse levelset info for '" << level_world << "': "
               << e.what() << std::endl;
    }
  }
  add_hl();
  add_back(_("Back"));
}

void
EditorDeleteLevelsetMenu::menu_action(MenuItem& item)
{
  int id = item.get_id();
  if (id >= 0)
  {
    if (Editor::is_active() && Editor::current()->get_world()->get_basedir() == m_editor_levelset_select_menu->m_contrib_worlds[id])
      Dialog::show_message(_("You cannot delete world that you are editing"));
    else
    {
      physfsutil::remote_with_content(m_editor_levelset_select_menu->m_contrib_worlds[id]);
      m_editor_levelset_select_menu->reload_menu();
      refresh();
    }
  }
}

/* EOF */