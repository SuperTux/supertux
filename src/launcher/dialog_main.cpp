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

#include "launcher/dialog_main.hpp"

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/levelset.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/textscroller_screen.hpp"
#include "supertux/world.hpp"
#include "util/log.hpp"
#include "util/file_system.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

#if defined(_WIN32)
  #include <windows.h>
  #include <shellapi.h>
#else
  #include <cstdlib>
#endif

LauncherMainMenu::LauncherMainMenu()
{
  set_center_pos(static_cast<float>(SCREEN_WIDTH) / 2.0f,
                 static_cast<float>(SCREEN_HEIGHT) / 2.0f + 35.0f);

  add_entry(MNID_LAUNCH, _("Launch SuperTux"));
  add_entry(MNID_QUITMAINMENU, _("Quit"));
}

void
LauncherMainMenu::on_window_resize()
{
  set_center_pos(static_cast<float>(SCREEN_WIDTH) / 2.0f,
                 static_cast<float>(SCREEN_HEIGHT) / 2.0f + 35.0f);
}

void
LauncherMainMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {

    case MNID_LAUNCH:
      log_warning << "This has not yet been implemented!" << std::endl;
      break;

    case MNID_QUITMAINMENU:
      MenuManager::instance().clear_menu_stack();
      ScreenManager::current()->quit(/*std::unique_ptr<ScreenFade>(new FadeToBlack(FadeToBlack::FADEOUT, 0.25))*/);
      break;
  }
}

/* EOF */
