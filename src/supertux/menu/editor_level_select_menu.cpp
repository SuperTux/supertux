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
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/levelset.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorLevelSelectMenu::EditorLevelSelectMenu() :
  m_world(),
  m_levelset()
{
  m_world = std::move(Editor::current()->world);
  initialize();
}

EditorLevelSelectMenu::EditorLevelSelectMenu(std::unique_ptr<World> world) :
  m_world(),
  m_levelset()
{
  m_world = std::move(world);
  initialize();
}

void EditorLevelSelectMenu::initialize() {
  auto editor = Editor::current();
  auto basedir = m_world->get_basedir();

  editor->deactivate_request = true;
  m_levelset = std::unique_ptr<Levelset>(new Levelset(basedir));

  add_label(m_world->get_title());
  add_hl();

  for (int i = 0; i < m_levelset->get_num_levels(); ++i)
  {
    std::string filename = m_levelset->get_level_filename(i);
    std::string full_filename = FileSystem::join(basedir, filename);
    std::string title = GameManager::current()->get_level_name(full_filename);
    add_entry(i, title);
  }

  add_hl();
  add_entry(-1,_("Create Level"));
  add_submenu(_("Level subset properties"), MenuStorage::EDITOR_LEVELSET_MENU, -3);
  add_back(_("Back"),-2);
}

EditorLevelSelectMenu::~EditorLevelSelectMenu()
{
  Editor::current()->world = std::move(m_world);
  Editor::current()->reactivate_request = true;
}

void
EditorLevelSelectMenu::create_level()
{
  Editor::current()->set_worldmap_mode(false);
  auto new_level = LevelParser::from_nothing(Editor::current()->get_world()->get_basedir());
  new_level->save(Editor::current()->get_world()->get_basedir() + "/" + new_level->filename);
  Editor::current()->set_level(new_level->filename);
  MenuManager::instance().clear_menu_stack();

  std::unique_ptr<Dialog> dialog(new Dialog);
  dialog->set_text(_("Share this level under license CC-BY-SA (advertized).\nIf you don't agree with this license, change it in level properties."));
  dialog->clear_buttons();
  dialog->add_button(_("OK"), [] {});
  MenuManager::instance().set_dialog(std::move(dialog));
}

void
EditorLevelSelectMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    Editor::current()->set_level(m_levelset->get_level_filename(item->id));
    Editor::current()->set_worldmap_mode(false);
    MenuManager::instance().clear_menu_stack();
  } else {
    switch (item->id) {
      case -1:
        create_level();
        break;
      case -2:
        MenuManager::instance().pop_menu();
        break;
      default:
        break;
    }
  }
}

/* EOF */
