//  SuperTux
//  Copyright (C) 2018 Tobias Markus <tobbi@supertux.org>
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

#include "supertux/menu/editor_layers_menu.hpp"

#include <boost/format.hpp>
#include <sstream>

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "editor/layer_icon.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorLayersMenu::EditorLayersMenu()
{
  add_label(_("Layers"));
  add_hl();

  int id = 0;
  for(auto& layer : Editor::current()->layerselect.layers) {
    auto layer_name = layer->layer->get_name();
    if(layer_name.empty())
    {
      layer_name = layer->layer->get_display_name();
    }
    auto z_pos = layer->get_zpos();
    add_icon_entry(id, str(boost::format(_("%s (%s)")) % layer_name % z_pos), layer->surface);
    id++;
  }

  add_hl();
  add_entry(-1, _("Add new layer"));

  add_hl();
  add_entry(-2,_("Cancel"));
}

EditorLayersMenu::~EditorLayersMenu()
{
  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  editor->reactivate_request = true;
}

void
EditorLayersMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    auto editor = Editor::current();
    editor->layerselect.selected_tilemap->editor_active = false;
    editor->layerselect.selected_tilemap = (TileMap*)(editor->layerselect.layers[item->id]->layer);
    editor->layerselect.selected_tilemap->editor_active = true;
  }
  if(item->id == -1)
  {
    MenuManager::instance().push_menu(MenuStorage::EDITOR_NEW_LAYER_MENU);
  }
  if(item->id == -2)
  {
    MenuManager::instance().pop_menu();
  }
}

/* EOF */
