//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/main_menu.hpp"

#include "audio/sound_manager.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/addon_menu.hpp"
#include "supertux/menu/contrib_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"

MainMenu::MainMenu()
{
  set_center_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 35);

  add_entry(MNID_STARTGAME, _("Start Game"));
  add_entry(MNID_LEVELS_CONTRIB, _("Contrib Levels"));
  add_entry(MNID_ADDONS, _("Add-ons"));
  add_submenu(_("Options"), MenuStorage::OPTIONS_MENU);
  add_entry(MNID_CREDITS, _("Credits"));
  add_entry(MNID_QUITMAINMENU, _("Quit"));
}

void
MainMenu::on_window_resize()
{
  set_center_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 35);
}

void
MainMenu::check_menu()
{
  switch (check())
  {
    case MNID_STARTGAME:
      {
        std::unique_ptr<World> world = World::load("levels/world1");
        GameManager::current()->start_game(std::move(world));
      }
      break;

    case MNID_LEVELS_CONTRIB:
      // Contrib Menu
      MenuManager::instance().push_menu(MenuStorage::CONTRIB_MENU);
      break;

    case MNID_ADDONS:
      // Add-ons Menu
      MenuManager::instance().push_menu(MenuStorage::ADDON_MENU);
      break;

    case MNID_CREDITS:
      MenuManager::instance().clear_menu_stack();
      g_screen_manager->push_screen(std::unique_ptr<Screen>(new TextScroller("credits.txt")),
                                    std::unique_ptr<ScreenFade>(new FadeOut(0.5)));
      break;

    case MNID_QUITMAINMENU:
      g_screen_manager->quit(std::unique_ptr<ScreenFade>(new FadeOut(0.25)));
      sound_manager->stop_music(0.25);
      break;
  }
}

/* EOF */
