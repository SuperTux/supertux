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

#include "supertux/menu/menu_storage.hpp"

#include "supertux/globals.hpp"
#include "supertux/menu/addon_menu.hpp"
#include "supertux/menu/contrib_menu.hpp"
#include "supertux/menu/contrib_world_menu.hpp"
#include "supertux/menu/game_menu.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "supertux/menu/language_menu.hpp"
#include "supertux/menu/main_menu.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/menu/profile_menu.hpp"
#include "supertux/menu/worldmap_menu.hpp"

MenuStorage* MenuStorage::s_instance = 0;

MenuStorage&
MenuStorage::instance()
{
  assert(s_instance);
  return *s_instance;
}

MenuStorage::MenuStorage()
{
  assert(!s_instance);
  s_instance = this;
}

MenuStorage::~MenuStorage()
{
  s_instance = nullptr;
}

std::unique_ptr<Menu>
MenuStorage::create(MenuId menu_id)
{
  switch(menu_id)
  {
    case MAIN_MENU:
      return std::unique_ptr<Menu>(new MainMenu);

    case LANGUAGE_MENU:
      return std::unique_ptr<Menu>(new LanguageMenu);

    case OPTIONS_MENU:
      return std::unique_ptr<Menu>(new OptionsMenu(true));

    case INGAME_OPTIONS_MENU:
      return std::unique_ptr<Menu>(new OptionsMenu(false));

    case PROFILE_MENU:
      return std::unique_ptr<Menu>(new ProfileMenu);

    case KEYBOARD_MENU:
      return std::unique_ptr<Menu>(new KeyboardMenu(g_input_manager));

    case JOYSTICK_MENU:
      return std::unique_ptr<Menu>(new JoystickMenu(g_input_manager));

    case WORLDMAP_MENU:
      return std::unique_ptr<Menu>(new WorldmapMenu);

    case GAME_MENU:
      return std::unique_ptr<Menu>(new GameMenu);

    case CONTRIB_MENU:
      return std::unique_ptr<Menu>(new ContribMenu);

    case CONTRIB_WORLD_MENU:
      return 0; //return new ContribWorldMenu();

    case ADDON_MENU:
      return std::unique_ptr<Menu>(new AddonMenu);

    case NO_MENU:
      return std::unique_ptr<Menu>();

    default:
      assert(!"unknown MenuId provided");
  }
}

KeyboardMenu*
MenuStorage::get_key_options_menu()
{
  assert(!"broken");
  return new KeyboardMenu(g_input_manager);
}

JoystickMenu*
MenuStorage::get_joystick_options_menu()
{
  assert(!"broken");
  return new JoystickMenu(g_input_manager);
}

/* EOF */
