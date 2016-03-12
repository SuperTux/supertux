//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "editor/object_menu.hpp"

#include "editor/editor.hpp"
#include "editor/object_settings.hpp"
#include "supertux/game_object.hpp"
#include "util/gettext.hpp"

ObjectMenu::ObjectMenu(GameObject *go) :
  object(go)
{
  ObjectSettings os = object->get_settings();
  add_label(os.name);
  add_hl();
  for(auto i = os.options.begin(); i != os.options.end(); ++i) {
    ObjectOption* oo = &(*i);
    switch (oo->type) {
      case MN_TEXTFIELD:
        add_textfield(oo->text, (std::string*)oo->option);
        break;
      case MN_NUMFIELD:
        add_numfield(oo->text, (float*)oo->option);
        break;
      case MN_INTFIELD:
        add_intfield(oo->text, (int*)oo->option);
        break;
      case MN_TOGGLE:
        add_toggle(-1, oo->text, (bool*)oo->option);
        break;
      case MN_STRINGSELECT:
        add_string_select(-1, oo->text, (size_t*)oo->option, oo->select);
        break;
      default:
        break;
    }
  }
  add_hl();
  add_back(_("OK"));
}

ObjectMenu::~ObjectMenu()
{
  object->after_editor_set();
  Editor::current()->reactivate_request = true;
}

void
ObjectMenu::menu_action(MenuItem* item)
{

}
