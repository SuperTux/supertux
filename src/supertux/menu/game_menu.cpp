//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/game_menu.hpp"

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "object/player.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "worldmap/worldmap.hpp"

GameMenu::GameMenu() :
  reset_callback ( [] {
    MenuManager::instance().clear_menu_stack();
    GameSession::current()->toggle_pause();
    GameSession::current()->reset_button = true;
  }),
  reset_checkpoint_callback( [] {
    MenuManager::instance().clear_menu_stack();
    GameSession::current()->toggle_pause();

    GameSession::current()->reset_checkpoint_button = true;
  }),
  abort_callback([] {
    GameSession::current()->abort_level();
  })
{
  Level& level = GameSession::current()->get_current_level();

  if (!level.get_name().empty())
  {
    add_label(level.get_name());
    add_hl();
  }

  add_entry(MNID_CONTINUE, _("Continue"));
  add_entry(MNID_RESETLEVEL, _("Restart Level"));

  if (Sector::current()->get_players()[0]->get_status().can_reach_checkpoint()) {
    add_entry(MNID_RESETLEVELCHECKPOINT, _("Restart from Checkpoint"));
  }

#ifndef HIDE_NONMOBILE_OPTIONS
  if (g_config->developer_mode && !Editor::current() &&
      // TODO: Allow to edit the level from a file; this is broken, so we don't
      //   show this button if there is no worldmap
      worldmap::WorldMap::current())
  {
    add_entry(MNID_EDITLEVEL, _("Edit Level"));
  }
#endif

  add_submenu(_("Options"), MenuStorage::INGAME_OPTIONS_MENU);
  add_hl();
  add_entry(MNID_ABORTLEVEL, _("Abort Level"));
}

void
GameMenu::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
    case MNID_CONTINUE:
      MenuManager::instance().clear_menu_stack();
      GameSession::current()->toggle_pause();
      break;

    case MNID_RESETLEVEL:
      if (g_config->confirmation_dialog)
      {
        Dialog::show_confirmation(_("Are you sure?"), reset_callback);
      }
      else
      {
        reset_callback();
      }
      break;

    case MNID_RESETLEVELCHECKPOINT:
      if (g_config->confirmation_dialog)
      {
        Dialog::show_confirmation(_("Are you sure?"),
                                  reset_checkpoint_callback);
      }
      else
      {
        reset_checkpoint_callback();
      }
      break;

    case MNID_EDITLEVEL:
      {
        if (Editor::is_active())
          break;

        if (!worldmap::WorldMap::current())
        {
          Dialog::show_message(_("Couldn't open editor for this level. No worldmap!"));
          break;
        }

        MenuManager::instance().clear_menu_stack();
        std::string level_file = GameSession::current()->get_level_file();
        std::string return_to = worldmap::WorldMap::current()->get_levels_path();
        // Pop ourselves out of the worldmap... don't ask :)
        ScreenManager::current()->pop_screen();
        ScreenManager::current()->pop_screen();
        // We must queue the creation of the level queue or else the currenton gets clobbered
        ScreenManager::current()->push_screen([level_file, return_to]() {
          Editor* editor = new Editor();
          if (level_file.empty())
            return editor;

          editor->set_level(FileSystem::basename(level_file));
          editor->set_world(World::from_directory(FileSystem::strip_leading_dirs(return_to)));
          editor->update(0, Controller());
          editor->on_exit([return_to]() {
            // Same as last comment... This restarts the previous level
            ScreenManager::current()->push_screen([return_to]() -> worldmap::WorldMap* {
              // TODO: Move this somewhere else, it is similar to the GameManager::start_worldmap code
              // Also, what if the world gets deleted in the middle of editing?
              std::unique_ptr<World> world = World::from_directory(FileSystem::strip_leading_dirs(return_to));
              if (!world)
              {
                Dialog::show_message(_("Couldn't open editor for this level."));
                return nullptr;
              }
              auto worldmap = GameManager::current()->create_worldmap_instance(*world);
              if (!worldmap)
              {
                Dialog::show_message(_("Couldn't open worldmap for this level."));
                return nullptr;
              }
              worldmap->start_level(true);
              return worldmap;
            });
          });

          return editor;
        });
      }
      break;

    case MNID_ABORTLEVEL:
      if (g_config->confirmation_dialog)
      {
        Dialog::show_confirmation(_("Are you sure?"), abort_callback);
      }
      else
      {
        abort_callback();
      }
      break;
  }
}
