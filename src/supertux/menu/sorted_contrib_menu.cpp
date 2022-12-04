//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
//                2022 Vankata453
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

#include "gui/item_action.hpp"
#include "supertux/levelset.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

SortedContribMenu::SortedContribMenu(std::vector<std::unique_ptr<World>>& worlds, const std::string& contrib_type,
                                     const std::string& title, const std::string& empty_message)
{
  add_label(title);
  add_hl();

  bool has_worldmap = false;
  int world_id = 0;
  for (unsigned int i = 0; i < worlds.size(); i++)
  {
    if (worlds[i]->get_contrib_type() == contrib_type)
    {
      std::string title_str;
      if (worlds[i]->is_levelset())
      {
        title_str = "[" + worlds[i]->get_title() + "]";
        m_world_entries.push_back({ false, worlds[i]->get_basedir(), nullptr, { -1, -1 } });
      }
      else
      {
        title_str = worlds[i]->get_title();
        has_worldmap = true;

        const std::string preview_file = FileSystem::join("previews", FileSystem::strip_extension(FileSystem::basename(worlds[i]->get_savegame_filename())) + ".png");
        SurfacePtr preview = find_preview(preview_file, worlds[i]->get_basedir());

        m_world_entries.push_back({ true, worlds[i]->get_basedir(), preview, Savegame::progress_from_file(worlds[i]->get_savegame_filename()) });
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

  // Adjust center position to give space for displaying previews.
  if (has_worldmap)
    set_center_pos(static_cast<float>(SCREEN_WIDTH) / 3,
                   static_cast<float>(SCREEN_HEIGHT) / 2);
}

/* EOF */
