//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/contrib_menu.hpp"

#include <physfs.h>
#include <sstream>

#include "gui/item_action.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "physfs/util.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/levelset.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "supertux/menu/sorted_contrib_menu.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

ContribMenu::ContribMenu() :
  m_contrib_worlds()
{
  // Generating contrib levels list by making use of Level Subset
  std::vector<std::string> level_worlds;

  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    files(PHYSFS_enumerateFiles("levels"),
          PHYSFS_freeList);
  for (const char* const* filename = files.get(); *filename != nullptr; ++filename)
  {
    std::string filepath = FileSystem::join("levels", *filename);
    if (physfsutil::is_directory(filepath))
    {
      level_worlds.push_back(filepath);
    }
  }

  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    addons(PHYSFS_enumerateFiles("custom"),
          PHYSFS_freeList);
  for (const char* const* addondir = addons.get(); *addondir != nullptr; ++addondir)
  {
    std::string addonpath = FileSystem::join("custom", *addondir);
    if (physfsutil::is_directory(addonpath))
    {
      std::string addonlevelpath = FileSystem::join(addonpath.c_str(), "levels");
      if (physfsutil::is_directory(addonlevelpath))
      {
        std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
          addonfiles(PHYSFS_enumerateFiles(addonlevelpath.c_str()),
                PHYSFS_freeList);
        for (const char* const* filename = addonfiles.get(); *filename != nullptr; ++filename)
        {
          std::string filepath = FileSystem::join(addonlevelpath.c_str(), *filename);
          if (physfsutil::is_directory(filepath))
          {
            level_worlds.push_back(filepath);
          }
        }
      }
    }
  }

  add_label(_("Contrib Levels"));
  add_hl();

  int i = 0;
  for (std::vector<std::string>::const_iterator it = level_worlds.begin(); it != level_worlds.end(); ++it)
  {
    try
    {
      auto levelset =
        std::unique_ptr<Levelset>(new Levelset(*it, /* recursively = */ true));
      if (levelset->get_num_levels() == 0)
        continue;

      std::unique_ptr<World> world = World::from_directory(*it);
      if (!world->hide_from_contribs())
      {
        if (world->is_levelset() || world->is_worldmap())
        {
          m_contrib_worlds.push_back(std::move(world));
        }
        else
        {
          log_warning << "unknown World type" << std::endl;
        }
      }
    }
    catch(std::exception& e)
    {
      log_info << "Couldn't parse levelset info for '" << *it << "': " << e.what() << std::endl;
    }
  }
  add_entry(0,_("Official Contrib Levels"));
  add_entry(1,_("Community Contrib Levels"));
  add_entry(2,_("User Contrib Levels"));
  add_hl();
  add_back(_("Back"));
}

void
ContribMenu::menu_action(MenuItem& item)
{
  int index = item.get_id();
  switch (index)
  {
  case 0: {
    auto contrib_menu = std::make_unique<SortedContribMenu>(m_contrib_worlds, "official", _("Official Contrib Levels"));
    MenuManager::instance().push_menu(std::move(contrib_menu));
    break;
  }
  case 1:{
    auto contrib_menu = std::make_unique<SortedContribMenu>(m_contrib_worlds, "community", _("Community Contrib Levels"));
    MenuManager::instance().push_menu(std::move(contrib_menu));
    break;
  }
  case 2:{
    auto contrib_menu = std::make_unique<SortedContribMenu>(m_contrib_worlds, "user", _("User Contrib Levels"));
    MenuManager::instance().push_menu(std::move(contrib_menu));
    break;
  }
  }
}

/* EOF */
