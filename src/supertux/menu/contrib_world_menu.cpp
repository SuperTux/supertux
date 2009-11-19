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

#include "supertux/menu/contrib_world_menu.hpp"

#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"

ContribWorldMenu::ContribWorldMenu(const World& current_world)
{
  add_label(current_world.title);
  add_hl();

  for (unsigned int i = 0; i < current_world.get_num_levels(); ++i)
  {
    /** get level's title */
    std::string filename = current_world.get_level_filename(i);
    std::string title = TitleScreen::get_level_name(filename);
    add_entry(i, title);
  }

  add_hl();
  add_back(_("Back"));
}

/* EOF */
