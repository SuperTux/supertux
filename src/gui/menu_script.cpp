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

#include "gui/menu_script.hpp"

#include "audio/sound_manager.hpp"
#include "gui/menu_item.hpp"
#include "gui/item_action.hpp"
#include "util/gettext.hpp"

ScriptMenu::ScriptMenu(std::string* script_) :
  base_script(script_)
{
  add_label(_("Edit the script"));
  add_hl();

  add_script_line(base_script);

  add_hl();
  add_back(_("OK"));
}

void
ScriptMenu::menu_action(MenuItem* item)
{

}

/* EOF */
