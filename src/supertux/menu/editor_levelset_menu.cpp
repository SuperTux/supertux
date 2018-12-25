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

#include "supertux/menu/editor_levelset_menu.hpp"

#include "gui/menu_item.hpp"
#include "editor/editor.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

EditorLevelsetMenu::EditorLevelsetMenu():
  world(Editor::current()->get_world()),
  levelset_type()
{
  initialize();
}

EditorLevelsetMenu::EditorLevelsetMenu(World* world_):
  world(world_),
  levelset_type()
{
  initialize();
}

EditorLevelsetMenu::~EditorLevelsetMenu()
{
  try
  {
    world->save();
  }
  catch(std::exception& e)
  {
    log_warning << "Could not save world: " << e.what() << std::endl;
  }
}

void
EditorLevelsetMenu::initialize() {

  levelset_type = world->is_levelset() ? 1 : 0;

  add_label(_("Level subset properties"));
  add_hl();
  add_textfield(_("Name"), &world->m_title);
  add_textfield(_("Description"), &world->m_description);
  add_string_select(1, _("Type"), &levelset_type, {_("Worldmap"), _("Levelset")});
  add_hl();
  add_back(_("OK"));
}

void
EditorLevelsetMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
  case 1:
    world->m_is_levelset = (levelset_type == 1);
    break;
  default:
    break;
  }
}

/* EOF */
