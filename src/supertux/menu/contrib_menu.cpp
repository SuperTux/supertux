//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"

ContribMenu::ContribMenu() :
  m_contrib_worlds()
{
  /** Generating contrib levels list by making use of Level Subset  */
  std::vector<std::string> level_worlds;
  char** files = PHYSFS_enumerateFiles("levels/");
  for(const char* const* filename = files; *filename != 0; ++filename) {
    std::string filepath = std::string("levels/") + *filename;
    if(PHYSFS_isDirectory(filepath.c_str()))
      level_worlds.push_back(filepath);
  }
  PHYSFS_freeList(files);

  add_label(_("Contrib Levels"));
  add_hl();

  int i = 0;
  for (std::vector<std::string>::const_iterator it = level_worlds.begin(); it != level_worlds.end(); ++it)
  {
    try
    {
      std::unique_ptr<World> world (new World());
      world->load(*it + "/info");
      if (!world->hide_from_contribs) 
      {
        add_entry(i++, world->get_title());
        m_contrib_worlds.push_back(std::move(world));
      }
    }
    catch(std::exception& e)
    {
      log_info << "Couldn't parse levelset info for '" << *it << "': " << e.what() << std::endl;
    }
  }

  add_hl();
  add_back(_("Back"));
}

ContribMenu::~ContribMenu()
{
}

void
ContribMenu::check_menu()
{
  int index = check();
  if (index != -1)
  {
    World* world = m_contrib_worlds[index].get();
    
    if (!world->is_levelset) 
    {
      TitleScreen::start_game(world);
    }
    else 
    {
#ifdef GRUMBEL
      m_contrib_world_menu.reset(new ContribWorldMenu(*world));
      MenuManager::instance().push_current(MenuStorage::CONTRIB_WORLD_MENU);
#endif
    }
  }
}

/* EOF */
