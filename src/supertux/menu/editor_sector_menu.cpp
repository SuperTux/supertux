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

#include "gui/menu_item.hpp"
#include "editor/editor.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"

EditorSectorMenu::EditorSectorMenu() :
  sector(Editor::current()->currentsector),
  original_name(sector->get_name()),
  size(sector->get_editor_size()),
  new_size(size),
  offset(0, 0)
{
  add_label(_("Sector") + " " + sector->get_name());
  add_hl();
  add_textfield(_("Name"), &sector->name);
  add_script(_("Initialization script"), &sector->init_script);
  add_color(_("Ambient light"), &sector->ambient_light);
  add_numfield(_("Gravity"), &sector->gravity);

  std::vector<std::string> music_formats;
  music_formats.push_back(".ogg");
  music_formats.push_back(".music");
  add_file(_("Music"), &sector->music, music_formats);

  add_hl();
  add_intfield(_("Width"), &(new_size.width));
  add_intfield(_("Height"), &(new_size.height));
  add_intfield(_("Resize offset X"), &(offset.width));
  add_intfield(_("Resize offset Y"), &(offset.height));
  add_entry(MNID_RESIZESECTOR, _("Resize"));

  add_hl();
  add_back(_("OK"));
}

EditorSectorMenu::~EditorSectorMenu()
{
  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  // Makes sure that the name of the sector isn't already used.
  auto level = editor->get_level();
  bool is_sector = false;
  for(auto const& sector_ : level->sectors) {
    if(sector_->get_name() == sector->get_name()) {
      if (is_sector) {
        // Puts the name that was there before when the name is already used.
        sector->set_name(original_name);
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
      if (new_size.is_valid()) {
        sector->resize_sector(size, new_size, offset);
        size = new_size;
      }
      break;
  }
}

/* EOF */
