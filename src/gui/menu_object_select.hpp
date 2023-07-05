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

#ifndef HEADER_SUPERTUX_GUI_MENU_OBJECT_SELECT_HPP
#define HEADER_SUPERTUX_GUI_MENU_OBJECT_SELECT_HPP

#include "gui/menu.hpp"

class GameObject;

class ObjectSelectMenu final : public Menu
{
public:
  ObjectSelectMenu(std::vector<std::unique_ptr<GameObject>>& objects, GameObject* parent);

  void refresh() override;
  void menu_action(MenuItem& item) override;

private:
  void add_object();
  void remove_object(GameObject* obj);

  const std::vector<std::string> get_available_objects() const;

private:
  std::vector<std::unique_ptr<GameObject>>& m_objects;
  GameObject* m_parent;

  std::string m_selected;

private:
  ObjectSelectMenu(const ObjectSelectMenu&) = delete;
  ObjectSelectMenu& operator=(const ObjectSelectMenu&) = delete;
};

#endif

/* EOF */
