//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
//                2023 Vankata453
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

#include "gui/item_object_select.hpp"

#include "gui/menu_manager.hpp"
#include "gui/menu_object_select.hpp"

ItemObjectSelect::ItemObjectSelect(const std::string& text, std::vector<std::unique_ptr<GameObject>>* objects,
                                   GameObject* parent, int id) :
  MenuItem(text, id),
  m_objects(objects),
  m_parent(parent)
{
}

void
ItemObjectSelect::process_action(const MenuAction& action)
{
  if (action == MenuAction::HIT)
    MenuManager::instance().push_menu(std::make_unique<ObjectSelectMenu>(*m_objects, m_parent));
}

/* EOF */
