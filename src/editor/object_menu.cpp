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
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/game_object.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

ObjectMenu::ObjectMenu(GameObject *go) :
  object(go)
{
  ObjectSettings os = object->get_settings();
  add_label(os.name);
  add_hl();
  for(auto& oo : os.options) {
    if(!(oo.flags & OPTION_VISIBLE)) {
      continue;
    }

    switch (oo.type) {
      case MN_TEXTFIELD:
        add_textfield(oo.text, (std::string*)oo.option);
        break;
      case MN_NUMFIELD:
        add_numfield(oo.text, (float*)oo.option);
        break;
      case MN_INTFIELD:
        add_intfield(oo.text, (int*)oo.option);
        break;
      case MN_TOGGLE:
        add_toggle(-1, oo.text, (bool*)oo.option);
        break;
      case MN_STRINGSELECT: {
        auto selected_id = (int*)oo.option;
        if ( *selected_id >= int(oo.select.size()) || *selected_id < 0 ) {
          *selected_id = 0; // Set the option to zero when not selectable
        }
        add_string_select(-1, oo.text, selected_id, oo.select);
      } break;
      case MN_BADGUYSELECT:
        add_badguy_select(oo.text, (std::vector<std::string>*)oo.option);
        break;
      case MN_COLOR:
        add_color(oo.text, (Color*)oo.option);
        break;
      case MN_SCRIPT:
        add_script(oo.text, (std::string*)oo.option);
        break;
      case MN_FILE:
        add_file(oo.text, (std::string*)oo.option, oo.select);
        break;
      case MN_REMOVE:
        add_entry(MNID_REMOVE, _("Remove"));
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

  auto editor = Editor::current();
  editor->reactivate_request = true;
  if (! dynamic_cast<MovingObject*>(object)) {
    editor->sort_layers();
  }
}

void
ObjectMenu::menu_action(MenuItem* item)
{
  switch (item->id) {
    case MNID_REMOVE:
      Editor::current()->delete_markers();
      Editor::current()->reactivate_request = true;
      MenuManager::instance().pop_menu();
      object->remove_me();
      break;
    default:
      break;
  }
}
