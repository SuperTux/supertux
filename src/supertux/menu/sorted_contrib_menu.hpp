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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_SORTED_CONTRIB_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_SORTED_CONTRIB_MENU_HPP

#include "supertux/menu/world_preview_menu.hpp"

class World;

class SortedContribMenu final : public WorldPreviewMenu
{
public:
  SortedContribMenu(std::vector<std::unique_ptr<World>>& worlds, const std::string& contrib_type,
                    const std::string& title, const std::string& empty_message);

private:
  SortedContribMenu(const SortedContribMenu&) = delete;
  SortedContribMenu& operator=(const SortedContribMenu&) = delete;
};

#endif

/* EOF */
