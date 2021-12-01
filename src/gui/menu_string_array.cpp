//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
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

#include "gui/menu_string_array.hpp"

#include "gui/menu_item.hpp"
#include "util/gettext.hpp"

StringArrayMenu::StringArrayMenu(std::vector<std::string>* items) :
  m_items(items),
  m_text()
{
  reload();
}

void
StringArrayMenu::menu_action(MenuItem& item)
{
  int id = item.get_id();
  if (id >= 0)
  {
    m_items->erase(m_items->begin() + id);
    reload();
  }
  else if (id == -2 && m_text.length() > 0)
  {
    m_items->push_back(m_text);
    m_text = "";
    reload();
  }
}

void
StringArrayMenu::reload()
{
  clear();
  add_label(_("Edit string array"));
  add_hl();
  for (unsigned int i = 0; i < m_items->size(); i++)
  {
    add_entry(i, m_items->at(i));
  }
  add_hl();
  add_textfield(_("Text"), &m_text);
  add_entry(-2, _("Add"));
  add_back(_("OK"));
}
/* EOF */
