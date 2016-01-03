//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "supertux/menu/editor_sector_menu.hpp"

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "editor/editor.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

EditorSectorMenu::EditorSectorMenu()
{
  add_label(_("Sector") + " " + Editor::current()->currentsector->get_name());
  add_hl();
  add_textfield(_("Name"), Editor::current()->currentsector->get_name_ptr());
  add_textfield(_("Initialization script"), Editor::current()->currentsector->get_init_script_ptr());
  add_color(_("Ambient light"), Editor::current()->currentsector->get_ambient_light_ptr());
  add_hl();
  add_back(_("OK"));
}

void
EditorSectorMenu::menu_action(MenuItem* item)
{

}

/* EOF */
