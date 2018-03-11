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

#include "supertux/menu/editor_new_layer_menu.hpp"

#include <boost/format.hpp>
#include <sstream>

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "editor/layer_icon.hpp"
#include "editor/object_menu.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorNewLayerMenu::EditorNewLayerMenu()
{
  add_label(_("Add new layer"));
  add_hl();

  add_entry(0, _("Tilemap"));
  add_entry(1, _("Background"));
  add_entry(2, _("Gradient"));
  add_entry(3, _("Thunderstorm"));
  add_entry(4, _("Cloud particle system"));
  add_entry(5, _("Rain particle system"));

  add_hl();
  add_entry(-2,_("Cancel"));
}

EditorNewLayerMenu::~EditorNewLayerMenu()
{
  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  editor->reactivate_request = true;
}

void
EditorNewLayerMenu::menu_action(MenuItem* item)
{
  auto editor = Editor::current();
  auto pos = Vector(0, 0);
  std::string object = "";
  if(item->id == 0)
  {
    object = "tilemap";
  }
  if(item->id == 1)
  {
    object = "background";
  }
  if(item->id == 2)
  {
    object = "gradient";
  }
  if(item->id == 3)
  {
    object = "thunderstorm";
  }
  if(item->id == 4)
  {
    object = "particles-clouds";
  }
  if(item->id == 5)
  {
    object = "particles-rain";
  }

  if(!object.empty())
  {
    auto obj = ObjectFactory::instance().create(object, pos, AUTO);
    if(obj == NULL)
    {
      log_warning << "Couldn't create object!" << std::endl;
      return;
    }
    editor->layerselect.add_layer(obj.get());
    editor->layerselect.sort_layers();
    try {
      editor->currentsector->add_object(obj);
    } catch(const std::exception& e) {
      log_warning << "Error adding object: " << e.what() << std::endl;
      return;
    }
    MenuManager::instance().pop_menu();

    if (item->id >= 0)
    {
      // Display object menu in which to set options for new layer
      std::unique_ptr<Menu> om(new ObjectMenu(obj.get()));
      MenuManager::instance().push_menu(move(om));
    }
  }
  if(item->id < 0)
  {
    MenuManager::instance().pop_menu();
  }
}

/* EOF */
