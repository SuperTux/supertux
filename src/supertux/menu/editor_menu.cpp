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
  bool is_temp_level = Editor::current()->is_temp_level();

  add_label(_("Level Editor"));
  add_hl();
  add_entry(MNID_RETURNTOEDITOR, _("Return to Editor"));
  add_entry(MNID_SAVELEVEL, worldmap ? _("Save Worldmap") : _("Save Level"));
  if (!worldmap) {
    add_entry(MNID_SAVEASLEVEL, _("Save Level as"));
    if (!is_temp_level)
      add_entry(MNID_SAVECOPYLEVEL, _("Save Copy"));
    add_entry(MNID_TESTLEVEL, _("Test Level"));
  } else {
    add_entry(MNID_TESTLEVEL, _("Test Worldmap"));
  }

  add_entry(MNID_OPTIONS, _("Options"));

  if (!is_temp_level)
  {
    add_entry(MNID_PACK, _("Package Add-On"));
    add_entry(MNID_OPEN_DIR, _("Open Level Directory"));
  }

  if (is_world && !is_temp_level)
    add_entry(MNID_LEVELSEL, _("Edit Another Level"));

  add_entry(MNID_LEVELSETSEL, _("Edit Another World"));

  if (!is_temp_level)
  {
    add_hl();

    add_submenu(_("Convert Tiles"), MenuStorage::EDITOR_CONVERTERS_MENU)
      .set_help(_("Convert all tiles in the level using converters."));
  }

  if (Editor::current()->has_deprecated_tiles())
  {
    add_hl();

    add_entry(MNID_CHECKDEPRECATEDTILES, _("Check for Deprecated Tiles"))
      .set_help(_("Check if any deprecated tiles are currently present in the level."));
    add_toggle(-1, _("Show Deprecated Tiles"), &(g_config->editor_show_deprecated_tiles))
      .set_help(_("Indicate all deprecated tiles on the active tilemap, without the need of hovering over."));
  }

  add_hl();

  add_submenu(_("Editor settings"), MenuStorage::EDITOR_SETTINGS_MENU);

  add_submenu(worldmap ? _("Worldmap Settings") : _("Level Settings"),
              MenuStorage::EDITOR_LEVEL_MENU);
  add_entry(MNID_HELP, _("Keyboard Shortcuts"));

  add_hl();
  if (!Editor::current()->is_temp_level())
    add_entry(MNID_CLOSELEVEL, _("Close Level"));
  else if (Editor::current()->has_unsaved_changes())
    add_entry(MNID_CLOSELEVEL, _("Reset level"));
  add_entry(MNID_QUITEDITOR, _("Exit Level Editor"));
}

EditorMenu::~EditorMenu()
{
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

  	case MNID_HELP:
    {
      auto dialog = std::make_unique<Dialog>();
      auto help_dialog_text =
        _("Keyboard Shortcuts") + ":\n" +
        "Esc = " + _("Open Menu") + "\n" +
        "Ctrl+S = " + _("Save") + "\n" +
        "Ctrl+T = " + _("Test") + "\n" +
        "Ctrl+Shift+T = " + _("Test at Cursor") + "\n" +
        "Ctrl+Alt+Shift+T = " + _("Test at Last Position") + "\n" +
        "Ctrl+Z = " + _("Undo") + "\n" +
        "Ctrl+Y = " + _("Redo") + "\n" +
        "F5 = " + _("Toggle Autotiling") + "\n" +
        "F6 = " + _("Render Light") + "\n" +
        "F7 = " + _("Grid Snapping") + "\n" +
        "F8 = " + _("Show Grid") + "\n" +
        "Ctrl+H = " + _("Hide Triggers") + "\n" +
        "Ctrl+X = " + _("Toggle Between Tileset/Objects Tool") + "\n" +
        _("Ctrl+PLUS or Ctrl+Scroll Up = Zoom In") + "\n" +
        _("Ctrl+MINUS or Ctrl+Scroll Down = Zoom Out") + "\n" +
        "Ctrl+D = " + _("Reset Zoom") + "\n\n" +
        _("Scripting Shortcuts") + ":\n" +
        _("Home = Go to beginning of line") + "\n" +
        _("End = Go to end of line") + "\n" +
        _("Left arrow = Go back in text") + "\n" +
        _("Right arrow = Go forward in text") + "\n" +
        _("Backspace = Delete in front of text cursor") + "\n" +
        _("Delete = Delete behind text cursor") + "\n" +
        "Ctrl+X = " + _("Cut whole line") + "\n" +
        "Ctrl+C = " + _("Copy whole line") + "\n" +
        "Ctrl+V = " + _("Paste") + "\n" +
        "Ctrl+D = " + _("Duplicate line") + "\n" +
        "Ctrl+Z = " + _("Undo") + "\n" +
        "Ctrl+Y = " + _("Redo");

      dialog->set_text(help_dialog_text);
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

    case MNID_CLOSELEVEL:
      editor->check_unsaved_changes([] {
        Editor::current()->set_level(nullptr, true);
        MenuManager::instance().clear_menu_stack();
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
