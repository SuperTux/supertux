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

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "editor/editor.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"

EditorLevelsetMenu::EditorLevelsetMenu()
{
  auto world = Editor::current()->get_world();

  add_label(_("Level subset properties"));
  add_hl();
  add_textfield(_("Name"), &(world->m_title));
  add_textfield(_("Description"), &(world->m_description));
  add_hl();
  add_back(_("OK"));
}

EditorLevelsetMenu::~EditorLevelsetMenu()
{
  Editor::current()->get_world()->save();
}

void
EditorLevelsetMenu::menu_action(MenuItem* item)
{

}

/* EOF */
