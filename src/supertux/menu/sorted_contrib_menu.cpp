//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
//                2022-2023 Vankata453
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
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

SortedContribMenu::SortedContribMenu(std::vector<std::unique_ptr<World>>& worlds, const std::string& contrib_type,
                                     const std::string& title, const std::string& empty_message)
{
  add_label(title);
  add_hl();

  bool has_worlds = false;
  for (const auto& world : worlds)
  {
    if (world->get_contrib_type() == contrib_type)
    {
      has_worlds = true;

      const auto savegame = Savegame::from_file(world->get_savegame_filename());

      ItemAction* item;
      if (world->is_levelset())
      {
        item = &add_world("[" + world->get_title() + "]", world->get_basedir(),
                          savegame->get_levelset_progress());
      }
      else
      {
        const std::string preview_file = FileSystem::join("previews", FileSystem::strip_extension(FileSystem::basename(world->get_savegame_filename())) + ".png");
        SurfacePtr preview = find_preview(preview_file, world->get_basedir());

        item = &add_world(world->get_title(), world->get_basedir(),
                          savegame->get_worldmap_progress(), preview);
      }
      item->set_help(world->get_description());
    }
  }
  if (!has_worlds)
  {
    add_inactive(empty_message);
  }
  add_hl();
  add_back(_("Back"));

  align_for_previews();
}

/* EOF */
