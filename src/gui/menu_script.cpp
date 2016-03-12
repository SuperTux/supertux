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
#include "gui/item_script_line.hpp"
#include "gui/menu_item.hpp"
#include "gui/item_action.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

ScriptMenu::ScriptMenu(std::string* script_) :
  base_script(script_),
  script_strings()
{
  script_strings.clear();

  add_label(_("Edit the script"));
  add_hl();

  // Split the script to the lines.
  std::string script = *base_script;
  std::string line_break = "\n";
  std::string new_line;
  size_t endl_pos = script.find(line_break);
  while (endl_pos != std::string::npos) {
    new_line = script.substr(0, endl_pos);
    script = script.substr(endl_pos + line_break.length());
    push_string(new_line);
    endl_pos = script.find(line_break);
  }
  push_string(script);

  //add_script_line(base_script);

  add_hl();
  add_back(_("OK"));
}

ScriptMenu::~ScriptMenu()
{
  *base_script = *(script_strings[0]);
  for (auto i = script_strings.begin()+1; i != script_strings.end(); ++i) {
    *base_script += "\n" + **i;
  }
}

void
ScriptMenu::push_string(std::string new_line)
{
  script_strings.push_back( move(std::unique_ptr<std::string>(new std::string(new_line))) );
  add_script_line( (script_strings.end()-1)->get() );
}

void
ScriptMenu::remove_line() {
  // The script should have at least one line.
  if (script_strings.size() <= 1) {
    return;
  }

  script_strings.erase(script_strings.begin() + (active_item - 2));
  delete_item(active_item);
}

void
ScriptMenu::add_line() {
  std::unique_ptr<std::string> new_line = std::unique_ptr<std::string>(new std::string(""));
  script_strings.insert(script_strings.begin() + (active_item - 1), move(new_line));

  std::unique_ptr<ItemScriptLine> line_item = std::unique_ptr<ItemScriptLine>(
        new ItemScriptLine( (script_strings.begin()+(active_item-1))->get() ));
  add_item(move(line_item), active_item+1);
  active_item++;
}

void
ScriptMenu::menu_action(MenuItem* item)
{

}

/* EOF */
