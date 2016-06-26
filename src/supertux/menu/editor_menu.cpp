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

#include "supertux/menu/editor_menu.hpp"

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "editor/editor.hpp"
#include "editor/input_center.hpp"
#include "editor/scroller.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

EditorMenu::EditorMenu()
{
  bool worldmap = Editor::current()->get_worldmap_mode();
  bool is_world = Editor::current()->get_world();
  std::vector<std::string> snap_grid_sizes;
  snap_grid_sizes.push_back("1/8 tile (4px)");
  snap_grid_sizes.push_back("1/4 tile (8px)");
  snap_grid_sizes.push_back("1/2 tile (16px)");
  snap_grid_sizes.push_back("1 tile (32px)");

  add_label(_("Level Editor"));
  add_hl();
  add_entry(MNID_RETURNTOEDITOR, _("Return to editor"));
  add_entry(MNID_SAVELEVEL, worldmap ? _("Save current worldmap") : _("Save current level"));

  if (!worldmap) {
    add_entry(MNID_TESTLEVEL, _("Test the level"));
  }

  if (is_world) {
    add_entry(MNID_LEVELSEL, _("Edit another level"));
  }

  add_entry(MNID_LEVELSETSEL, _("Choose another level subset"));

  add_string_select(-1, _("Grid size"), &EditorInputCenter::selected_snap_grid_size, snap_grid_sizes);

  add_toggle(-1, _("Render lighting (F6)"), &DrawingContext::render_lighting);
  add_toggle(-1, _("Snap objects to grid (F7)"), &EditorInputCenter::snap_to_grid);
  add_toggle(-1, _("Show grid (F8)"), &EditorInputCenter::render_grid);
  add_toggle(-1, _("Show scroller (F9)"), &EditorScroller::rendered);

  add_submenu(worldmap ? _("Worldmap properties") : _("Level properties"),
              MenuStorage::EDITOR_LEVEL_MENU);

  add_hl();
  add_entry(MNID_QUITEDITOR, _("Exit level editor"));
}

EditorMenu::~EditorMenu()
{
  Editor::current()->reactivate_request = true;
}

void
EditorMenu::menu_action(MenuItem* item)
{
  switch (item->id)
  {
    case MNID_RETURNTOEDITOR:
      MenuManager::instance().clear_menu_stack();
      break;

    case MNID_SAVELEVEL:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->save_request = true;
      break;

    case MNID_TESTLEVEL:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->test_request = true;
      break;

    case MNID_LEVELSEL:
      MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVEL_SELECT_MENU);
      break;

    case MNID_LEVELSETSEL:
      MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      break;

    case MNID_QUITEDITOR:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->quit_request = true;
      break;

    default:
      break;
  }
}

/* EOF */
