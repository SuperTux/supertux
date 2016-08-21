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

#include <physfs.h>
#include <sstream>

#include "editor/editor.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "supertux/menu/editor_level_select_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorLevelsetSelectMenu::EditorLevelsetSelectMenu() :
  m_contrib_worlds()
{
  Editor::current()->deactivate_request = true;
  // Generating contrib levels list by making use of Level Subset
  std::vector<std::string> level_worlds;

  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    files(PHYSFS_enumerateFiles("levels"),
          PHYSFS_freeList);
  for(const char* const* filename = files.get(); *filename != 0; ++filename)
  {
    std::string filepath = FileSystem::join("levels", *filename);
    PHYSFS_Stat statbuf;
    PHYSFS_stat(filepath.c_str(), &statbuf);
    if(statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY)
    {
      level_worlds.push_back(filepath);
    }
  }

  add_label(_("Choose level subset"));
  add_hl();

  int i = 0;
  for (const auto& level_world : level_worlds)
  {
    try
    {
      std::unique_ptr<World> world = World::load(level_world);

      if (!world->hide_from_contribs())
      {
        Savegame savegame(world->get_savegame_filename());
        savegame.load();

        if (world->is_levelset())
        {
          int level_count = 0;

          const auto& state = savegame.get_levelset_state(world->get_basedir());
          for(const auto& level_state : state.level_states)
          {
            if(level_state.filename == "")
              continue;
            level_count += 1;
          }

          std::ostringstream title;
          title << "[" << world->get_title() << "]";
          if (level_count == 0)
          {
            title << " " << _("*NEW*");
          }
          else
          {
            title << " (" << level_count << " " << _("levels") << ")";
          }
          add_entry(i++, title.str());
          m_contrib_worlds.push_back(level_world);
        }
        else if (world->is_worldmap())
        {
          int level_count = 0;

          const auto& state = savegame.get_worldmap_state(world->get_worldmap_filename());
          for(const auto& level_state : state.level_states)
          {
            if(level_state.filename == "")
              continue;
            level_count += 1;
          }

          std::ostringstream title;
          title << world->get_title();
          if (level_count == 0)
          {
            title << " " << _("*NEW*");
          }
          else
          {
            title << " (" << level_count << " " << _("levels") << ")";
          }
          add_entry(i++, title.str());
          m_contrib_worlds.push_back(level_world);
        }
        else
        {
          log_warning << "unknown World type" << std::endl;
        }
      }
    }
    catch(std::exception& e)
    {
      log_info << "Couldn't parse levelset info for '" << level_world << "': "
               << e.what() << std::endl;
    }
  }

  add_hl();
  add_submenu(_("New level subset"), MenuStorage::EDITOR_NEW_LEVELSET_MENU);
  add_back(_("Back"),-2);
}

EditorLevelsetSelectMenu::~EditorLevelsetSelectMenu()
{
  auto editor = Editor::current();
  if (!editor->levelloaded && !editor->reload_request) {
    editor->quit_request = true;
  } else {
    editor->reactivate_request = true;
  }
}

void
EditorLevelsetSelectMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    std::unique_ptr<Menu> menu = std::unique_ptr<Menu>(new EditorLevelSelectMenu(
                                 World::load(m_contrib_worlds[item->id])));
    MenuManager::instance().push_menu(std::move(menu));
  }
}

/* EOF */
