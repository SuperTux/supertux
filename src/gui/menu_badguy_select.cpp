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

#include "gui/menu_badguy_select.hpp"

#include <fmt/format.h>

#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_list.hpp"
#include "supertux/game_object_factory.hpp"
#include "editor/editor.hpp"
BadguySelectMenu::BadguySelectMenu(std::vector<std::string>* badguys_) :
  badguys(badguys_),
  selected(),
  remove_item()
{
  refresh();
}

void
BadguySelectMenu::refresh()
{
  m_items.clear();

  add_label(_("List of enemies"));
  add_hl();
  add_entry(-2, fmt::format(_("Select enemy ({})"), selected));
  add_entry(-3, _("Add"));
  add_hl();

  int i = 0;
  for (auto& badguy : *badguys) {
    add_entry(i, badguy);
    i++;
  }

  add_hl();
  add_back(_("OK"));
}

void
BadguySelectMenu::remove_badguy()
{
  badguys->erase(badguys->begin() + remove_item);
  refresh();
  if (m_items[m_active_item]->skippable()) {
    //We are on the bottom headline.
    m_active_item++;
  }
}

void
BadguySelectMenu::add_badguy()
{
  if (selected == "")
  {
    log_warning << "Cannot add an empty enemy." << std::endl;
    return;
  }
    badguys->push_back(selected);
    refresh();
  
}

void
BadguySelectMenu::menu_action(MenuItem& item)
{
  if (item.get_id() >= 0) {
    remove_item = item.get_id();
    auto self  = this;
    // confirmation dialog
    auto dialog = std::make_unique<Dialog>();
    dialog->set_text(_("Do you want to delete this badguy from the list?"));
    dialog->add_default_button(_("Yes"), [self] {
      self->remove_badguy();
    });
    dialog->add_cancel_button(_("No"));
    MenuManager::instance().set_dialog(std::move(dialog));
  } else if (item.get_id() == -2) {
    MenuManager::instance().push_menu(std::make_unique<ListMenu>(GameObjectFactory::instance().get_registered_badguys(), &selected, this));
  } else if (item.get_id() == -3) {
    add_badguy();
  }
}

/* EOF */
