//  SuperTux
//  Copyright (C) 2026 SuperTux Team
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

#include "supertux/menu/global_stats_menu.hpp"

#include <memory>

#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen/global_stats_screen.hpp"
#include "supertux/screen_manager.hpp"
#include "util/gettext.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

GlobalStatsMenu::GlobalStatsMenu()
{
  add_label(_("Global Statistics"));
  add_hl();
  add_entry(MNID_VIEW_GLOBAL_STATS, _("View Statistics"));
  add_hl();
  add_back(_("Back"));

  on_window_resize();
}

void
GlobalStatsMenu::on_window_resize()
{
  set_center_pos(static_cast<float>(SCREEN_WIDTH) / 2.0f,
                 static_cast<float>(SCREEN_HEIGHT) / 2.0f + 35.0f);
}

void
GlobalStatsMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
    case MNID_VIEW_GLOBAL_STATS:
    {
      MenuManager::instance().clear_menu_stack();
      auto screen = std::make_unique<GlobalStatsScreen>();
      auto fade = std::make_unique<FadeToBlack>(FadeToBlack::FADEOUT, 0.25f);
      ScreenManager::current()->push_screen(std::move(screen), std::move(fade));
      break;
    }
  }
}
