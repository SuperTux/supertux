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
#include "editor/layer_icon.hpp"
#include "editor/object_settings.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/game_object.hpp"
#include "supertux/object_factory.hpp"
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
  if(object != NULL)
  {
    object->after_editor_set();
  }

  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  editor->reactivate_request = true;
  if (! dynamic_cast<MovingObject*>(object)) {
    MenuManager::instance().refresh_menu_stack();
    editor->sort_layers();
  }
}

void
ObjectMenu::menu_action(MenuItem* item)
{
  switch (item->id) {
    case MNID_REMOVE:
    {
      auto editor = Editor::current();
      // TODO: Having the delete option in the object menu is a 
      // *really* bad idea, UI-wise and otherwise.
      auto obj_as_tilemap = dynamic_cast<TileMap*>(object);
      TileMap *previous = NULL, *next = NULL;
      bool found = false, next_found = false;
      if( obj_as_tilemap && editor->layerselect.selected_tilemap == obj_as_tilemap)
      {
        // Let's get this layer and return the previous one:
        for(const auto& layer : editor->layerselect.layers)
        {
          if(dynamic_cast<TileMap*>(layer->layer))
          {
            if(layer->layer == obj_as_tilemap)
            {
              found = true;
            }
            else
            {
              next_found = true;
              if(!found)
              {
                previous = (TileMap*)(layer->layer);
              }
              else if(found && next == NULL)
              {
                next = (TileMap*)(layer->layer);
              }
            }
          }
        }
        if(!next_found)
        {
          std::unique_ptr<Dialog> dialog(new Dialog);
          dialog->set_text(_("There must be at least one tilemap in each sector."));
          dialog->clear_buttons();
          dialog->add_button(_("OK"), [] {});
          MenuManager::instance().set_dialog(std::move(dialog));
          return;
        }
        else
        {
          if(previous != NULL)
          {
            editor->layerselect.selected_tilemap = previous;
          }
          else if(next != NULL)
          {
            editor->layerselect.selected_tilemap = next;
          }
        }
      }
      object->remove_me();
      object = NULL;

      editor->delete_markers();
      editor->reactivate_request = true;
      MenuManager::instance().refresh_menu_stack();
      MenuManager::instance().pop_menu();
    }
      break;
    default:
      break;
  }
}
