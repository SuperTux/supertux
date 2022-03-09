//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
//                2021-2022 Jiri Palecek <narre@protonmail.com>
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

#include <sstream>

#include "boost/format.hpp"

#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "gui/item_action.hpp"
#include "supertux/savegame.hpp"
#include "supertux/player_status.hpp"
#include "supertux/levelset.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "util/gettext.hpp"

SortedContribMenu::SortedContribMenu(std::vector<std::unique_ptr<World>>& worlds, const std::string& contrib_type, const std::string& title, const std::string& empty_message) :
  m_world_folders()
{ 
  add_label(title);
  add_hl();
  int world_id = 0;
  for (unsigned int i = 0; i < worlds.size(); i++)
  {
    if (worlds[i]->get_contrib_type() == contrib_type)
    {
      m_world_folders.push_back(worlds[i]->get_basedir());
      std::string title_str;
      const auto savegame = Savegame::from_file(worlds[i]->get_savegame_filename());

      if (worlds[i]->is_levelset())
      {
        const auto& level_count = savegame->get_levelset_state(worlds[i]->get_basedir()).get_level_count();

        if (!level_count.second)
        {
          title_str = str(boost::format(_("[%s] *NEW*")) % worlds[i]->get_title());
        }
        else
        {
          title_str = str(boost::format(_("[%s] (%u/%u; %u%%)")) % worlds[i]->get_title() %
                          level_count.first % level_count.second % (100 * level_count.first / level_count.second));
        }
      }
      else
      {
        std::string wm_filename = savegame->get_player_status().last_worldmap;

        if (wm_filename.empty())
          wm_filename = worlds[i]->get_worldmap_filename();

        wm_filename = "/" + wm_filename;

        std::pair<uint32_t, uint32_t> world_level_count;
        for (const auto& world : savegame->get_worldmaps())
          world_level_count.first += savegame->get_worldmap_state(world).get_level_count().first;
        const auto levelset = std::unique_ptr<Levelset>(new Levelset(worlds[i]->get_basedir(), true));
        world_level_count.second = levelset->get_num_levels();

        const auto& island_level_count = savegame->get_worldmap_state(wm_filename).get_level_count();

        if (!island_level_count.second && !world_level_count.first)
          title_str = str(boost::format(_("%s *NEW*")) % worlds[i]->get_title());
        else
        {
          const std::string wm_title = savegame->get_player_status().last_worldmap_title;

          if (island_level_count.second == world_level_count.second)
          {
            const uint32_t percentage = 100 * island_level_count.first / island_level_count.second;
            title_str = str(boost::format(_("%s (%u/%u; %u%%)")) % worlds[i]->get_title() %
                            island_level_count.first % island_level_count.second % percentage);
          }
          else
          {
            const uint32_t percentage = island_level_count.second ? (100 * island_level_count.first / island_level_count.second) : 100;
            title_str = str(boost::format(_("%s (%u/%u; %u%%) - %s (%u/%u; %u%%)")) % worlds[i]->get_title() % 
                            world_level_count.first % world_level_count.second %
                            (100 * world_level_count.first / world_level_count.second) %
                            wm_title % island_level_count.first % island_level_count.second %
                            percentage);
          }
        }
      }
      add_entry(world_id++, title_str).set_help(worlds[i]->get_description());
    }
  }
  if (world_id == 0)
  {
    add_inactive(empty_message);
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
