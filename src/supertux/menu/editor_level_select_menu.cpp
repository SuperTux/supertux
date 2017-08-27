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
#include "supertux/menu/editor_levelset_menu.hpp"
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
  m_levelset()
{
  initialize();
}

EditorLevelSelectMenu::EditorLevelSelectMenu(std::unique_ptr<World> world) :
  m_levelset()
{
  Editor::current()->world = std::move(world);
  initialize();
}

void EditorLevelSelectMenu::initialize() {
  auto editor = Editor::current();
  auto& world = editor->world;
  auto basedir = world->get_basedir();
  editor->deactivate_request = true;
  m_levelset = std::unique_ptr<Levelset>(new Levelset(basedir, /* recursively = */ true));
  auto num_levels = m_levelset->get_num_levels();

  add_label(world->get_title());
  add_hl();

  if(num_levels == 0)
  {
    add_inactive(_("Empty levelset"));
  }
  else
  {
    for (int i = 0; i < num_levels; ++i)
    {
      std::string filename = m_levelset->get_level_filename(i);
      std::string full_filename = FileSystem::join(basedir, filename);
      std::string title = GameManager::current()->get_level_name(full_filename);
      add_entry(i, title);
    }
  }

  add_hl();

  std::string worldmap_file = FileSystem::join(basedir, "worldmap.stwm");
  if (PHYSFS_exists(worldmap_file.c_str())) {
    add_entry(-4, _("Edit worldmap"));
  } else {
    add_entry(-5, _("Create worldmap"));
  }
  add_hl();

  add_entry(-1, _("Create Level"));
  add_entry(-3, _("Level subset properties"));
  add_back(_("Back"),-2);
}

EditorLevelSelectMenu::~EditorLevelSelectMenu()
{
  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  editor->reactivate_request = true;
}

void
EditorLevelSelectMenu::create_level()
{
  Editor::current()->set_worldmap_mode(false);
  create_item();
}

void
EditorLevelSelectMenu::create_worldmap()
{
  Editor::current()->set_worldmap_mode(true);
  create_item();
}

void
EditorLevelSelectMenu::create_item()
{
  auto editor = Editor::current();
  auto& world = editor->world;
  auto basedir = world->get_basedir();
  auto new_item = editor->get_worldmap_mode() ?
      LevelParser::from_nothing_worldmap(basedir, world->m_title) :
      LevelParser::from_nothing(basedir);
  new_item->save(basedir + "/" + new_item->filename);
  editor->set_level(new_item->filename);
  MenuManager::instance().clear_menu_stack();

  std::unique_ptr<Dialog> dialog(new Dialog);
  if(editor->get_worldmap_mode())
  {
    dialog->set_text(_("Share this worldmap under license CC-BY-SA 4.0 International (advised).\nIt allows modifications and redistribution by third-parties.\nIf you don't agree with this license, change it in worldmap properties.\nDISCLAIMER: The SuperTux authors take no responsibility for your choice of license."));
  }
  else
  {
    dialog->set_text(_("Share this level under license CC-BY-SA 4.0 International (advised).\nIt allows modifications and redistribution by third-parties.\nIf you don't agree with this license, change it in level properties.\nDISCLAIMER: The SuperTux authors take no responsibility for your choice of license."));
  }
  dialog->clear_buttons();
  dialog->add_button(_("OK"), [] {});
  MenuManager::instance().set_dialog(std::move(dialog));
}

void
EditorLevelSelectMenu::menu_action(MenuItem* item)
{
  auto editor = Editor::current();
  auto& world = editor->world;
  if (item->id >= 0)
  {
    editor->set_level(m_levelset->get_level_filename(item->id));
    editor->set_worldmap_mode(false);
    MenuManager::instance().clear_menu_stack();
  } else {
    switch (item->id) {
      case -1:
        create_level();
        break;
      case -2:
        MenuManager::instance().pop_menu();
        break;
      case -3: {
        auto menu = std::unique_ptr<Menu>(new EditorLevelsetMenu(world.get()));
        MenuManager::instance().push_menu(std::move(menu));
      } break;
      case -4:
        editor->set_level("worldmap.stwm");
        editor->set_worldmap_mode(true);
        MenuManager::instance().clear_menu_stack();
        break;
      case -5:
        create_worldmap();
        break;
      default:
        break;
    }
  }
}

/* EOF */
