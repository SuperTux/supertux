//  SuperTux
//  Copyright (C) 2022 mrkubax10 <mrkubax10@onet.pl>
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

#include "supertux/menu/video_system_menu.hpp"

#include <fmt/format.h>

#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "supertux/globals.hpp"
#include "supertux/gameconfig.hpp"
#include "util/gettext.hpp"
#include "video/video_system.hpp"

VideoSystemMenu::VideoSystemMenu()
{
  refresh();
}

void
VideoSystemMenu::refresh()
{
  add_label(_("Select Video System"));
  add_inactive(fmt::format(_("Used video system: {}"), VideoSystem::get_video_string(g_config->video)));
  add_hl();

  std::vector<std::string> video_systems = VideoSystem::get_available_video_systems();
  for(unsigned i = 0; i < video_systems.size(); i++)
    add_entry(static_cast<int>(i), video_systems[i]);

  add_hl();
  add_back(_("Back"));
}

void
VideoSystemMenu::menu_action(MenuItem& item)
{
  if (item.get_id() >= 0)
  {
    g_config->video = VideoSystem::get_video_system(item.get_text());
    Dialog::show_message(_("Restart game for the changes to take effect"), false, false, []
      {
        MenuManager::instance().pop_menu();
      });
  }
}

/* EOF */
