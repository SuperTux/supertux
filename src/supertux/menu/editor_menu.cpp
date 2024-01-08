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

#include <physfs.h>
#include <fmt/format.h>

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/item_action.hpp"
#include "gui/item_goto.hpp"
#include "gui/item_toggle.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/level.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/editor_save_as.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"
#include "video/compositor.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

EditorMenu::EditorMenu()
{
  refresh();
}

void
EditorMenu::refresh()
{
  clear();

  bool worldmap = Editor::current()->get_level()->is_worldmap();
  bool is_world = Editor::current()->get_world() != nullptr;
  std::vector<std::string> snap_grid_sizes;
  snap_grid_sizes.push_back(_("tiny tile (4px)"));
  snap_grid_sizes.push_back(_("small tile (8px)"));
  snap_grid_sizes.push_back(_("medium tile (16px)"));
  snap_grid_sizes.push_back(_("big tile (32px)"));

  const bool editing_remote_level = Editor::current()->is_editing_remote_level();

  add_label(_("Level Editor"));
  add_hl();

  if (Editor::current()->is_hosting_level())
  {
    const network::Address address = Editor::current()->get_server_address();
    const size_t peers = Editor::current()->get_connected_peers();
    add_inactive(fmt::format(fmt::runtime(_("Hosting level on port {}")), address.port));
    add_inactive(fmt::format(fmt::runtime(__("{} peer connected", "{} peers connected", static_cast<int>(peers))),
                             peers));
    add_hl();
  }
  else if (editing_remote_level)
  {
    network::Address address = Editor::current()->get_server_address();
    add_inactive(fmt::format(fmt::runtime(_("Connected to {}:{}")),
                 address.host, address.port));
    add_hl();
  }

  add_entry(MNID_RETURNTOEDITOR, _("Return to Editor"));
  if (!editing_remote_level)
    add_entry(MNID_SAVELEVEL, worldmap ? _("Save Worldmap") : _("Save Level"));
  if (!worldmap) {
    if (!editing_remote_level)
      add_entry(MNID_SAVEASLEVEL, _("Save Level as"));
    add_entry(MNID_SAVECOPYLEVEL, _("Save Copy"));
    add_entry(MNID_TESTLEVEL, _("Test Level"));
  } else {
    add_entry(MNID_TESTLEVEL, _("Test Worldmap"));
  }

  add_entry(MNID_OPTIONS, _("Options"));

  add_entry(MNID_SHARE, _("Share Level"));

  if (!editing_remote_level)
  {
    add_entry(MNID_PACK, _("Package Add-On"));

    add_entry(MNID_OPEN_DIR, _("Open Level Directory"));

    if (Editor::current()->is_hosting_level())
      add_entry(MNID_STOP_HOSTING_LEVEL, _("Stop Hosting Level"))
        .set_help(_("Stop hosting the current level over the network."));
    else
      add_submenu(_("Host Level"), MenuStorage::EDITOR_HOST_LEVEL_MENU)
        .set_help(_("Host the current level over the network."));
  }
  else
  {
    add_entry(MNID_RELOAD_LEVEL, _("Reload Level"))
      .set_help(_("Reloads the current level from the remote server."));
  }

  if (is_world)
    add_entry(MNID_LEVELSEL, _("Edit Another Level"));

  add_entry(MNID_LEVELSETSEL, _("Edit Another World"));

  add_hl();

  add_submenu(_("Convert Tiles"), MenuStorage::EDITOR_CONVERTERS_MENU)
    .set_help(_("Convert all tiles in the level using converters."));

  add_hl();

  add_string_select(-1, _("Grid Size"), &(g_config->editor_selected_snap_grid_size), snap_grid_sizes);
  add_toggle(-1, _("Show Grid"), &(g_config->editor_render_grid));
  add_toggle(-1, _("Grid Snapping"), &(g_config->editor_snap_to_grid));
  add_toggle(-1, _("Render Background"), &(g_config->editor_render_background));
  add_toggle(-1, _("Render Light"), &(Compositor::s_render_lighting));
  add_toggle(-1, _("Autotile Mode"), &(g_config->editor_autotile_mode));
  add_toggle(-1, _("Enable Autotile Help"), &(g_config->editor_autotile_help));
  add_toggle(-1, _("Enable Object Undo Tracking"), &(g_config->editor_undo_tracking));
  if (g_config->editor_undo_tracking)
  {
    add_intfield(_("Undo Stack Size"), &(g_config->editor_undo_stack_size), -1, true);
  }
  add_intfield(_("Autosave Frequency"), &(g_config->editor_autosave_frequency));

  if (Editor::current()->has_deprecated_tiles())
  {
    add_hl();

    add_entry(MNID_CHECKDEPRECATEDTILES, _("Check for Deprecated Tiles"))
      .set_help(_("Check if any deprecated tiles are currently present in the level."));
    add_toggle(-1, _("Show Deprecated Tiles"), &(g_config->editor_show_deprecated_tiles))
      .set_help(_("Indicate all deprecated tiles on the active tilemap, without the need of hovering over."));
  }

  add_hl();

  if (!editing_remote_level)
    add_submenu(worldmap ? _("Worldmap Settings") : _("Level Settings"),
                MenuStorage::EDITOR_LEVEL_MENU);
  add_entry(MNID_HELP, _("Keyboard Shortcuts"));

  add_hl();
  add_entry(MNID_QUITEDITOR, _("Exit Level Editor"));
}

EditorMenu::~EditorMenu()
{
  auto editor = Editor::current();

  if (editor == nullptr)
    return;

  editor->m_reactivate_request = true;
}

void
EditorMenu::menu_action(MenuItem& item)
{
  auto editor = Editor::current();
  switch (item.get_id())
  {
    case MNID_RETURNTOEDITOR:
      MenuManager::instance().clear_menu_stack();
      break;

    case MNID_SAVELEVEL:
    {
      editor->check_save_prerequisites([editor]() {
        MenuManager::instance().clear_menu_stack();
        editor->m_save_request = true;
      });
    }
      break;

    case MNID_SAVEASLEVEL:
    {
      editor->check_save_prerequisites([] {
        MenuManager::instance().set_menu(std::make_unique<EditorSaveAs>(true));
      });
    }
      break;

    case MNID_SAVECOPYLEVEL:
    {
      editor->check_save_prerequisites([] {
        MenuManager::instance().set_menu(std::make_unique<EditorSaveAs>(false));
      });
    }
      break;

    case MNID_PACK:
      Dialog::show_confirmation(_("Do you want to package this world as an add-on?"), [] {
        Editor::current()->pack_addon();
        FileSystem::open_path(FileSystem::join(PHYSFS_getWriteDir(), "addons"));
      });
      break;

    case MNID_OPEN_DIR:
      Editor::current()->open_level_directory();
      break;

    case MNID_TESTLEVEL:
    {
      editor->check_save_prerequisites([editor]() {
        MenuManager::instance().clear_menu_stack();
        editor->m_test_pos = std::nullopt;
        editor->m_test_request = true;
      });
    }
      break;

    case MNID_OPTIONS:
      MenuManager::instance().push_menu(MenuStorage::OPTIONS_MENU);
      break;

    case MNID_SHARE:
    {
      Dialog::show_confirmation(_("We encourage you to share your levels in the SuperTux forum.\nTo find your level, click the\n\"Open Level directory\" menu item.\nDo you want to go to the forum now?"), [] {
        FileSystem::open_url("https://forum.freegamedev.net/viewforum.php?f=69");
      });
    }
    break;

    case MNID_STOP_HOSTING_LEVEL:
      Editor::current()->stop_hosting_level();
      refresh();
      break;

    case MNID_RELOAD_LEVEL:
      Dialog::show_confirmation(_("Reloading would remove any local changes, not available on the server.") + "\n \n" +
                                _("Are you sure?"),
        []() {
          Editor::current()->reload_remote_level();
        });
      break;

    case MNID_HELP:
    {
      auto dialog = std::make_unique<Dialog>();
      dialog->set_text(_("Keyboard Shortcuts:\n---------------------\nEsc = Open Menu\nCtrl+S = Save\nCtrl+T = Test\nCtrl+Z = Undo\nCtrl+Y = Redo\nF5 = Toggle Autotiling\nF6 = Render Light\nF7 = Grid Snapping\nF8 = Show Grid\nCtrl++ or Ctrl+Scroll Up = Zoom In\nCtrl+- or Ctrl+Scroll Down = Zoom Out\nCtrl+D = Reset Zoom\n\nScripting Shortcuts:\n    -------------    \nHome = Go to beginning of line\nEnd = Go to end of line\nLeft arrow = Go back in text\nRight arrow = Go forward in text\nBackspace = Delete in front of text cursor\nDelete = Delete behind text cursor\nCtrl+X = Cut whole line\nCtrl+C = Copy whole line\nCtrl+V = Paste\nCtrl+D = Duplicate line\nCtrl+Z = Undo\nCtrl+Y = Redo"));
      dialog->add_cancel_button(_("Got it!"));
      MenuManager::instance().set_dialog(std::move(dialog));
    }
    break;

    case MNID_LEVELSEL:
      editor->check_unsaved_changes([] {
        MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVEL_SELECT_MENU);
      });
      break;

    case MNID_LEVELSETSEL:
      editor->check_unsaved_changes([] {
        MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      });
      break;

    case MNID_QUITEDITOR:
      MenuManager::instance().clear_menu_stack();
      Editor::current()->m_quit_request = true;
      break;

    case MNID_CHECKDEPRECATEDTILES:
      editor->check_deprecated_tiles(true);
      if (editor->has_deprecated_tiles())
      {
        const std::string present_message = _("Deprecated tiles are still present in the level.");
        if (g_config->editor_show_deprecated_tiles)
        {
          Dialog::show_message(present_message);
        }
        else
        {
          Dialog::show_confirmation(present_message + "\n\n" + _("Do you want to show all deprecated tiles on active tilemaps?"), []() {
            g_config->editor_show_deprecated_tiles = true;
          });
        }
      }
      else
      {
        Dialog::show_message(_("There are no more deprecated tiles in the level!"));
        refresh();
      }
      break;

    default:
      break;
  }
}

bool
EditorMenu::on_back_action()
{
  auto editor = Editor::current();
  if (!editor)
    return true;

  editor->retoggle_undo_tracking();
  editor->undo_stack_cleanup();

  return true;
}

/* EOF */
