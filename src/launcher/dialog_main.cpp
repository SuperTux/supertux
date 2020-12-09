//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
//     (copied from supertux/menu/main_menu.hpp)
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "launcher/video_system.hpp"
#include "supertux/main.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "video/viewport.hpp"

#if defined(_WIN32)
  #include <windows.h>
  #include <shellapi.h>
#else
  #include <cstdlib>
#endif

LauncherMainMenu::LauncherMainMenu(char* arg0) :
  m_arg0(arg0)
{
  set_center_pos(static_cast<float>(SCREEN_WIDTH) / 2.0f,
                 static_cast<float>(SCREEN_HEIGHT) / 2.0f + 35.0f);

  add_entry(MNID_LAUNCH, _("Launch SuperTux"));
  add_entry(MNID_LAUNCH_OTHER, _("Launch other SuperTux"));
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
    int result;
    case MNID_LAUNCH:
      result = Main().run(0, (char**) m_arg0);
      log_warning << "Game exited with: " << result << std::endl;
      break;

    case MNID_LAUNCH_OTHER:
      result = system("supertux2");
      log_warning << "Game exited with: " << result << std::endl;
      break;

    case MNID_QUITMAINMENU:
      MenuManager::instance().clear_menu_stack();
      // I have to #include "supertux/screen_fade.hpp" because this line doesn't
      // know how to build a null unique pointer from an undefined class.
      // And yes, I will die mad about it.   ~Semphris
      ScreenManager::current()->quit();
      break;
  }
}
/* EOF */