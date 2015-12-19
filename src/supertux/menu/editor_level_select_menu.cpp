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

#include "supertux/menu/editor_level_select_menu.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/levelset.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorLevelSelectMenu::EditorLevelSelectMenu() :
  m_levelset()
{
  m_levelset = std::unique_ptr<Levelset>(new Levelset(Editor::current()->world->get_basedir()));

  add_label(Editor::current()->world->get_title());
  add_hl();

  for (int i = 0; i < m_levelset->get_num_levels(); ++i)
  {
    std::string filename = m_levelset->get_level_filename(i);
    std::string full_filename = FileSystem::join(Editor::current()->world->get_basedir(), filename);
    std::string title = GameManager::current()->get_level_name(full_filename);
    add_entry(i, title);
  }

  add_hl();
  add_entry(-1,_("Create Level"));
  add_entry(-2,_("Back"));
}

void
EditorLevelSelectMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    Editor::current()->levelfile = m_levelset->get_level_filename(item->id);
    Editor::current()->reload_request = true;
    MenuManager::instance().clear_menu_stack();
  } else {
    MenuManager::instance().pop_menu();
  }
}

/* EOF */
