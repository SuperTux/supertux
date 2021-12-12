//  SuperTux -- List menu
//  Copyright (C) 2021 Rami <rami.slicer@gmail.com>
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

#include "gui/menu_item.hpp"
#include "gui/menu_list.hpp"
#include "gui/menu_manager.hpp"

ListMenu::ListMenu(const std::vector<std::string>& items, int* selected, Menu* parent) : 
  m_selected(selected),
  m_parent(parent)
{
  for(size_t i = 0; i < items.size(); i++) {
    add_entry(static_cast<int>(i), items[i]);
  }
  add_hl();
  add_back("OK");
}

void
ListMenu::menu_action(MenuItem& item) {
  if(m_selected) {
    *m_selected = item.get_id();
  }
  m_parent->refresh();
  MenuManager::instance().pop_menu();
}

/* EOF */
