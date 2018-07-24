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
#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/textscroller.hpp"

MainMenu::MainMenu()
{
  set_center_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 35);

  add_entry(MNID_STARTGAME, _("Start Game"));
  add_entry(MNID_ADDONS, _("Add-ons"));
  add_submenu(_("Options"), MenuStorage::OPTIONS_MENU);
  add_entry(MNID_LEVELEDITOR, _("Level Editor"));
  add_entry(MNID_CREDITS, _("Credits"));
  add_entry(MNID_QUITMAINMENU, _("Quit"));
}

void
MainMenu::on_window_resize()
{
  set_center_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 35);
}

void
MainMenu::menu_action(MenuItem* item)
{
  switch (item->id)
  {

    case MNID_STARTGAME:
      // World selection menu
      MenuManager::instance().push_menu(MenuStorage::WORLDSET_MENU);
      break;

    case MNID_ADDONS:
      // Add-ons Menu
      MenuManager::instance().push_menu(MenuStorage::ADDON_MENU);
      break;


    case MNID_CREDITS:
      MenuManager::instance().clear_menu_stack();
      ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new TextScroller("credits.stxt")),
                                            std::unique_ptr<ScreenFade>(new FadeOut(0.5)));
      break;

    case MNID_LEVELEDITOR:
      {
        MenuManager::instance().clear_menu_stack();
        std::unique_ptr<Screen> screen(new Editor());
        std::unique_ptr<FadeOut> fade(new FadeOut(0.5));
        SoundManager::current()->stop_music(0.5);
        ScreenManager::current()->push_screen(move(screen),move(fade));
        //Editor::current()->setup();
      }
      break;

    case MNID_QUITMAINMENU:
      MenuManager::instance().clear_menu_stack();
      ScreenManager::current()->quit(std::unique_ptr<ScreenFade>(new FadeOut(0.25)));
      SoundManager::current()->stop_music(0.25);
      break;
  }
}

/* EOF */
