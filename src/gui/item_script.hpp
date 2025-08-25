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

#pragma once

#include "gui/menu_item.hpp"

class ItemScript final : public MenuItem
{
public:
  ItemScript(UID uid, const std::string& key, const std::string& text_, std::string* script_, int id = -1);

  /** Processes the menu action. */
  virtual void process_action(const MenuAction& action) override;

private:
  std::string* script;
  std::string m_key;
  UID m_uid;

private:
  ItemScript(const ItemScript&) = delete;
  ItemScript& operator=(const ItemScript&) = delete;
};
