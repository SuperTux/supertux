//  SuperTux
//  Copyright (C) 2024
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

#include "supertux/menu/character_selector_menu.hpp"

#include "gui/menu_manager.hpp"
#include "util/gettext.hpp"

CharacterSelectorMenu::CharacterSelectorMenu(
  const std::function<void(int)>& on_select,
  const std::function<void()>& on_cancel)
  : m_on_select(on_select), m_on_cancel(on_cancel)
{
  add_label(_("Select Character"));
  add_hl();

  add_entry(_("Tux"), [this] {
    if (m_on_select)
      m_on_select(CHARACTER_TUX);
  });

  add_entry(_("Penny"), [this] {
    if (m_on_select)
      m_on_select(CHARACTER_PENNY);
  });

  add_entry(_("Larry"), [this] {
    if (m_on_select)
      m_on_select(CHARACTER_LARRY);
  });

  add_hl();
  add_back(_("Cancel"));
}

bool CharacterSelectorMenu::on_back_action()
{
  if (m_on_cancel)
    m_on_cancel();
  return true;  // Allow menu to pop
}
