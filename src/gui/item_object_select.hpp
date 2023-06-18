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

#ifndef HEADER_SUPERTUX_GUI_ITEM_OBJECT_SELECT_HPP
#define HEADER_SUPERTUX_GUI_ITEM_OBJECT_SELECT_HPP

#include "gui/menu_item.hpp"

class GameObject;

class ItemObjectSelect final : public MenuItem
{
public:
  ItemObjectSelect(const std::string& text, std::vector<std::unique_ptr<GameObject>>* objects,
                   GameObject* parent, int id = -1);

  /** Processes the menu action. */
  virtual void process_action(const MenuAction& action) override;

private:
  std::vector<std::unique_ptr<GameObject>>* m_objects;
  GameObject* m_parent;

private:
  ItemObjectSelect(const ItemObjectSelect&) = delete;
  ItemObjectSelect& operator=(const ItemObjectSelect&) = delete;
};

#endif

/* EOF */
