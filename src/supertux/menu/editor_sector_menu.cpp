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
#include "supertux/level.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

EditorSectorMenu::EditorSectorMenu() :
  sector(Editor::current()->currentsector),
  sector_name_ptr(sector->get_name_ptr()),
  original_name(*sector_name_ptr),
  size(sector->get_editor_size()),
  new_size(size)
{
  add_label(_("Sector") + " " + sector->get_name());
  add_hl();
  add_textfield(_("Name"), sector_name_ptr);
  add_script(_("Initialization script"), sector->get_init_script_ptr());
  add_color(_("Ambient light"), sector->get_ambient_light_ptr());
  add_numfield(_("Gravity"), &sector->gravity);

  add_file(_("Music"), &sector->music, std::vector<std::string>(1, ".ogg"));

  add_hl();
  add_intfield(_("Width"), &(new_size.width));
  add_intfield(_("Height"), &(new_size.height));
  add_entry(MNID_RESIZESECTOR, _("Resize"));

  add_hl();
  add_back(_("OK"));
}

EditorSectorMenu::~EditorSectorMenu()
{
  // Makes sure that the name of the sector isn't already used.
  Level* level = Editor::current()->get_level();
  bool is_sector = false;
  for(auto const& sector_ : level->sectors) {
    if(sector_->get_name() == sector->get_name()) {
      if (is_sector) {
        // Puts the name that was there before when the name is already used.
        *sector_name_ptr = original_name;
        break;
      } else {
        is_sector = true;
      }
    }
  }
}

void
EditorSectorMenu::menu_action(MenuItem* item)
{
  switch (item->id) {
    case MNID_RESIZESECTOR:
      if (new_size.width > 0 && new_size.height > 0) {
        sector->resize_sector(size, new_size);
        size = new_size;
      }
      break;
  }
}

/* EOF */
