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
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/game_object.hpp"

ObjectMenu::ObjectMenu(Editor& editor, GameObject* go) :
  m_editor(editor),
  m_object(go)
{
  ObjectSettings os = m_object->get_settings();
  add_label(os.get_name());
  add_hl();
  for (const auto& oo : os.get_options()) {
    if (!(oo.m_flags & OPTION_VISIBLE)) {
      continue;
    }

    switch (oo.m_type) {
      case MN_TEXTFIELD:
        add_textfield(oo.m_text, static_cast<std::string*>(oo.m_option));
        break;
      case MN_NUMFIELD:
        add_numfield(oo.m_text, static_cast<float*>(oo.m_option));
        break;
      case MN_INTFIELD:
        add_intfield(oo.m_text, static_cast<int*>(oo.m_option));
        break;
      case MN_TOGGLE:
        add_toggle(-1, oo.m_text, static_cast<bool*>(oo.m_option));
        break;
      case MN_STRINGSELECT:
        {
          auto selected_id = static_cast<int*>(oo.m_option);
          if ( *selected_id >= static_cast<int>(oo.m_select.size()) || *selected_id < 0 ) {
            *selected_id = 0; // Set the option to zero when not selectable
          }
          add_string_select(-1, oo.m_text, selected_id, oo.m_select);
        }
        break;

      case MN_BADGUYSELECT:
        add_badguy_select(oo.m_text, static_cast<std::vector<std::string>*>(oo.m_option));
        break;

      case MN_COLOR:
        add_color(oo.m_text, static_cast<Color*>(oo.m_option));
        break;

      case MN_SCRIPT:
        add_script(oo.m_text, static_cast<std::string*>(oo.m_option));
        break;

      case MN_FILE:
        add_file(oo.m_text, static_cast<std::string*>(oo.m_option), oo.m_select);
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
  m_object->after_editor_set();

  m_editor.m_reactivate_request = true;
  if (!dynamic_cast<MovingObject*>(m_object)) {
    m_editor.sort_layers();
  }
}

void
ObjectMenu::menu_action(MenuItem& item)
{
  switch (item.get_id()) {
    case MNID_REMOVE:
      m_editor.delete_markers();
      m_editor.m_reactivate_request = true;
      MenuManager::instance().pop_menu();
      m_object->remove_me();
      break;
    default:
      break;
  }
}

/* EOF */
