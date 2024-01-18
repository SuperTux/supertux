//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "supertux/menu/editor_remote_level_menu.hpp"

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/item_intfield.hpp"
#include "gui/menu_manager.hpp"

EditorRemoteLevelMenu::EditorRemoteLevelMenu(bool connect) :
  m_connect(connect),
  m_host_address(),
  m_port(),
  m_nickname(),
  m_nickname_color(1, 1, 1, 1)
{
  add_label(m_connect ? _("Edit Remote Level") : _("Host Level"));
  add_hl();

  if (m_connect)
    add_textfield(_("Host Address"), &m_host_address);

  auto& port_field = add_intfield(_("Port"), &m_port, -1, true);
  if (!m_connect)
  {
    port_field.set_help(_("Port number must be between 1024 and 65535.\nSet to 0 for random port."));
  }
  else
  {
    add_textfield(_("Nickname"), &m_nickname)
      .set_help(_("Nickname character count must be between 3 and 20."));
    add_color(_("Nickname Color"), &m_nickname_color, false);

    add_hl();
  }

  add_entry(1, m_connect ? _("Connect") : _("Host"));

  add_hl();
  add_back(_("Back"));
}

void
EditorRemoteLevelMenu::menu_action(MenuItem& item)
{
  if (item.get_id() != 1) return;

  if (m_port != 0 && (m_port < 1024 || m_port > 65535))
  {
    Dialog::show_message(_("Port number must be between 1024 and 65535."));
    return;
  }

  if (m_connect)
  {
    Editor::current()->set_remote_level(m_host_address, static_cast<uint16_t>(m_port),
                                        m_nickname, m_nickname_color);
  }
  else
  {
    Editor::current()->host_level(static_cast<uint16_t>(m_port));

    MenuManager::instance().pop_menu();
    MenuManager::instance().current_menu()->refresh();
  }
}

/* EOF */
