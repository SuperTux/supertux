//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
//                2021 Jiri Palecek <narre@protonmail.com>
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
        uint32_t level_count = 0, solved_count = 0;
        const auto& state = savegame->get_levelset_state(worlds[i]->get_basedir());

        for (const auto& level_state : state.level_states)
        {
          if (level_state.filename.empty() || level_state.filename.back() == '~') continue;
          if (level_state.solved) ++solved_count;
          ++level_count;
        }

        if (!level_count)
        {
          title_str = str(boost::format(_("[%s] *NEW*")) % worlds[i]->get_title());
        }
        else
        {
          title_str = str(boost::format(_("[%s] (%u/%u; %u%%)")) % worlds[i]->get_title() %
                          solved_count % level_count % (100 * solved_count / level_count));
        }
      }
      else
      {
        uint32_t island_level_count = 0, island_solved_count = 0,
                 world_level_count = 0,  world_solved_count = 0;
        std::string wm_filename = savegame->get_player_status().last_worldmap;

        if (wm_filename.empty())
          wm_filename = worlds[i]->get_worldmap_filename();

        wm_filename = "/" + wm_filename;

        const auto& state = savegame->get_worldmap_state(wm_filename);
        for (const auto& level_state : state.level_states)
        {
          if (level_state.filename.empty()) continue;
          if (level_state.solved) ++island_solved_count;
          ++island_level_count;
        }

        const auto levelset = std::unique_ptr<Levelset>(new Levelset(worlds[i]->get_basedir(), true));
        world_level_count = levelset->get_num_levels();
        const auto world_list = savegame->get_worldmaps();

        for (const auto& world : world_list)
        {
          const auto& world_state = savegame->get_worldmap_state(world);
          for (const auto& level_state : world_state.level_states)
          {
            if (level_state.filename.empty()) continue;
            if (level_state.solved) ++world_solved_count;
          }
        }

        if (!island_level_count && !world_solved_count)
          title_str = str(boost::format(_("%s *NEW*")) % worlds[i]->get_title());
        else
        {
          const auto wm_title = savegame->get_player_status().last_worldmap_title;

          if (island_level_count == world_level_count)
          {
            title_str = str(boost::format(_("%s (%u/%u; %u%%)")) % worlds[i]->get_title() %
                            island_solved_count % island_level_count % (100 * island_solved_count / island_level_count));
          }
          else
          {
            title_str = str(boost::format(_("%s (%u/%u; %u%%) - %s (%u/%u; %u%%)")) % worlds[i]->get_title() % 
                            world_solved_count % world_level_count % (100 * world_solved_count / world_level_count) %
                            wm_title % island_solved_count % island_level_count %
                            (island_level_count ? (100 * island_solved_count / island_level_count) : 100));
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
