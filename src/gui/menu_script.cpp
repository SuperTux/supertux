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

#include "gui/menu_script.hpp"

#include "gui/item_scriptfield.hpp"
#include "util/gettext.hpp"

ScriptMenu::ScriptMenu(std::string* script)
{
  add_label(_("Edit script"));
  add_hl();

  auto scriptbox = std::make_unique<ControlTextbox>(true);
  scriptbox->bind_string(script);
  MenuItem& item = add_item(std::make_unique<ItemScriptField>(std::move(scriptbox), Sizef(0.6f, 0.6f)));

  add_hl();
  add_back(_("OK"));

  item.process_action(MenuAction::SELECT);
}

/* EOF */
