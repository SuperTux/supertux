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

#include "gui/menu_object_select.hpp"

#include <fmt/format.h>

#include "badguy/dispenser.hpp"
#include "editor/object_menu.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_list.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/game_object_factory.hpp"
#include "util/log.hpp"

ObjectSelectMenu::ObjectSelectMenu(std::vector<std::unique_ptr<GameObject>>& objects, GameObject* parent) :
  m_objects(objects),
  m_parent(parent),
  m_selected()
{
  refresh();
}

void
ObjectSelectMenu::refresh()
{
  clear();

  add_label(_("List of objects"));
  add_hl();
  add_entry(-2, fmt::format(fmt::runtime(_("Select object ({})")),
      m_selected.empty() ? "" : GameObjectFactory::instance().get_display_name(m_selected)));
  add_entry(-3, _("Add"));
  add_hl();

  int i = 0;
  for (auto& obj : m_objects)
  {
    add_entry(i, obj->get_display_name());
    i++;
  }

  add_hl();
  add_back(_("OK"));
}

void
ObjectSelectMenu::add_object()
{
  if (m_selected.empty())
  {
    log_warning << "Cannot add object: No object selected." << std::endl;
    return;
  }

  auto obj = GameObjectFactory::instance().create(m_selected);
  obj->update_version(); // Ensure the object is on its latest version

  Dispenser* dispenser = dynamic_cast<Dispenser*>(m_parent);
  if (dispenser)
    dispenser->add_object(std::move(obj)); // The Dispenser itself should add the object, running appropriate checks
  else
    m_objects.push_back(std::move(obj));

  refresh();
}

void
ObjectSelectMenu::remove_object(GameObject* obj)
{
  m_objects.erase(
    std::remove_if(m_objects.begin(), m_objects.end(), [obj](const auto& found_obj)
    {
      return obj == found_obj.get();
    }), m_objects.end());
  refresh();
}

const std::vector<std::string>
ObjectSelectMenu::get_available_objects() const
{
  if (dynamic_cast<Dispenser*>(m_parent))
    return GameObjectFactory::instance().get_registered_objects(ObjectFactory::RegisteredObjectParam::OBJ_PARAM_DISPENSABLE);

  return GameObjectFactory::instance().get_registered_objects();
}

void
ObjectSelectMenu::menu_action(MenuItem& item)
{
  if (item.get_id() >= 0)
  {
    MenuManager::instance().push_menu(std::make_unique<ObjectMenu>(m_objects[item.get_id()].get(),
        [this](GameObject* obj) {
          Dialog::show_confirmation(_("Are you sure you want to remove this object from the list?"), [this, obj]() {
            remove_object(obj);
            MenuManager::instance().pop_menu();
          });
          return false;
        }));
  }
  else if (item.get_id() == -2)
  {
    MenuManager::instance().push_menu(std::make_unique<ListMenu>(get_available_objects(),
        &m_selected, this, [](const std::string& obj_name) {
          return GameObjectFactory::instance().get_display_name(obj_name);
        }));
  }
  else if (item.get_id() == -3)
  {
    add_object();
  }
}

/* EOF */
