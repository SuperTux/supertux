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

#include "supertux/menu/sorted_contrib_menu.hpp"

#include "util/gettext.hpp"
#include "supertux/savegame.hpp"
#include "supertux/player_status.hpp"
#include "supertux/levelset.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
SortedContribMenu::SortedContribMenu(std::vector<std::unique_ptr<World>>& worlds, std::string contrib_type, std::string title) : 
  m_world_folders()
{ 
  add_label(title);
  add_hl();
  int world_id = 0;
  for (unsigned int i = 0; i < worlds.size(); i++)
  {
    if (worlds[i]->get_contrib_type() == contrib_type)
    {
      auto savegame = Savegame::from_file(worlds[i]->get_savegame_filename());
      if (worlds[i]->is_levelset())
      {
        m_world_folders.push_back(worlds[i]->get_basedir());
        std::ostringstream title;
        title << "[" << worlds[i]->get_title() << "]";
        add_entry(world_id++, title.str());
      }
      else if (worlds[i]->is_worldmap())
      {
        m_world_folders.push_back(worlds[i]->get_basedir());
        std::ostringstream title;
        title << worlds[i]->get_title();
        add_entry(world_id++,title.str());
      }
    }
  }
  add_hl();
  add_back(_("Back"));
}
void
SortedContribMenu::menu_action(MenuItem& item)
{
  int index = item.get_id();
  if (index >= 0)
  {
    std::unique_ptr<World> world = World::from_directory(m_world_folders[index]);
    if (world->is_levelset())
    {
      MenuManager::instance().push_menu(std::unique_ptr<Menu>(new ContribLevelsetMenu(std::move(world))));
    }
    else
    {
      GameManager::current()->start_worldmap(*world);
    }
  }
}
/* EOF */
