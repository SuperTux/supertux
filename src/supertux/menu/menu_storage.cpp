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

#include "supertux/menu/menu_storage.hpp"

#include <assert.h>

#include "supertux/menu/addon_menu.hpp"
#include "supertux/menu/addon_browse_menu.hpp"
#include "supertux/menu/cheat_menu.hpp"
#include "supertux/menu/debug_menu.hpp"
#include "supertux/menu/contrib_menu.hpp"
#include "supertux/menu/custom_menu_menu.hpp"
#include "supertux/menu/editor_converters_menu.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "supertux/menu/editor_level_menu.hpp"
#include "supertux/menu/editor_level_select_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/menu/editor_new_levelset_menu.hpp"
#include "supertux/menu/editor_objectgroup_menu.hpp"
#include "supertux/menu/editor_tilegroup_menu.hpp"
#include "supertux/menu/editor_tilesubgroup_menu.hpp"
#include "supertux/menu/editor_sector_menu.hpp"
#include "supertux/menu/editor_sectors_menu.hpp"
#include "supertux/menu/game_menu.hpp"
#include "supertux/menu/integrations_menu.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "supertux/menu/language_menu.hpp"
#include "supertux/menu/main_menu.hpp"
#include "supertux/menu/multiplayer_menu.hpp"
#include "supertux/menu/multiplayer_players_menu.hpp"
#include "supertux/menu/options_select_menu.hpp"
#include "supertux/menu/particle_editor_menu.hpp"
#include "supertux/menu/particle_editor_save_as.hpp"
#include "supertux/menu/particle_editor_open.hpp"
#include "supertux/menu/profile_menu.hpp"
#include "supertux/menu/video_system_menu.hpp"
#include "supertux/menu/web_asset_menu.hpp"
#include "supertux/menu/worldmap_menu.hpp"
#include "supertux/menu/worldmap_cheat_menu.hpp"
#include "supertux/menu/world_set_menu.hpp"
#include "util/log.hpp"

MenuStorage* MenuStorage::s_instance = nullptr;

MenuStorage&
MenuStorage::instance()
{
  assert(s_instance);
  return *s_instance;
}

MenuStorage::MenuStorage()
{
  assert(!s_instance);
  s_instance = this;
}

MenuStorage::~MenuStorage()
{
  s_instance = nullptr;
}

std::unique_ptr<Menu>
MenuStorage::create(MenuId menu_id)
{
  switch (menu_id)
  {
    case MAIN_MENU:
      return std::make_unique<MainMenu>();

    case LANGUAGE_MENU:
      return std::make_unique<LanguageMenu>();

    case OPTIONS_MENU:
      return std::make_unique<OptionsSelectMenu>(true);

    case INGAME_OPTIONS_MENU:
      return std::make_unique<OptionsSelectMenu>(false);

    case PROFILE_MENU:
      return std::make_unique<ProfileMenu>();

    case KEYBOARD_MENU:
      return std::unique_ptr<Menu>(new KeyboardMenu(*InputManager::current()));

    case JOYSTICK_MENU:
      return std::unique_ptr<Menu>(new JoystickMenu(*InputManager::current()));

    case VIDEO_SYSTEM_MENU:
      return std::make_unique<VideoSystemMenu>();

    case WORLDMAP_MENU:
      return std::make_unique<WorldmapMenu>();

    case WORLDMAP_CHEAT_MENU:
      return std::make_unique<WorldmapCheatMenu>();

    case WORLDMAP_LEVEL_SELECT_MENU:
      return std::make_unique<WorldmapLevelSelectMenu>();

    case GAME_MENU:
      return std::make_unique<GameMenu>();

    case CHEAT_MENU:
      return std::make_unique<CheatMenu>();

    case DEBUG_MENU:
      return std::make_unique<DebugMenu>();

    case WORLDSET_MENU:
      return std::make_unique<WorldSetMenu>();

    case CONTRIB_MENU:
      return std::make_unique<ContribMenu>();

    case CONTRIB_WORLD_MENU:
      return nullptr; //return new ContribWorldMenu();

    case ADDON_MENU:
      return std::make_unique<AddonMenu>();

    case LANGPACK_MENU:
      return std::unique_ptr<Menu>(new AddonMenu(true));

    case EDITOR_LEVELSET_SELECT_MENU:
      return std::make_unique<EditorLevelsetSelectMenu>();

    case EDITOR_NEW_LEVELSET_MENU:
      return std::make_unique<EditorNewLevelsetMenu>();

    case LANGPACK_AUTO_UPDATE_MENU:
      return std::unique_ptr<Menu>(new AddonBrowseMenu(true, true));

    case EDITOR_LEVEL_SELECT_MENU:
      return std::make_unique<EditorLevelSelectMenu>();

    case EDITOR_MENU:
      return std::make_unique<EditorMenu>();

    case EDITOR_TILEGROUP_MENU:
      return std::make_unique<EditorTilegroupMenu>();
    
    case EDITOR_TILESUBGROUP_MENU_SNOW:
      return std::make_unique<EditorTilesubgroupMenu>("Snow");

    case EDITOR_TILESUBGROUP_MENU_SNOW_BACKGROUND:
      return std::make_unique<EditorTilesubgroupMenu>("Snow Background");

    case EDITOR_TILESUBGROUP_MENU_CRYSTAL:
      return std::make_unique<EditorTilesubgroupMenu>("Crystal");

    case EDITOR_TILESUBGROUP_MENU_FOREST:
      return std::make_unique<EditorTilesubgroupMenu>("Forest");

    case EDITOR_TILESUBGROUP_MENU_FOREST_BACKGROUND:
      return std::make_unique<EditorTilesubgroupMenu>("Forest Background");

    case EDITOR_TILESUBGROUP_MENU_CORRUPTED_FOREST:
      return std::make_unique<EditorTilesubgroupMenu>("Corrupted Forest");

    case EDITOR_TILESUBGROUP_MENU_CORRUPTED_BACKGROUND:
      return std::make_unique<EditorTilesubgroupMenu>("Corrupted Background");

    case EDITOR_TILESUBGROUP_MENU_JAGGED_ROCKS:
      return std::make_unique<EditorTilesubgroupMenu>("Jagged Rocks");

    case EDITOR_TILESUBGROUP_MENU_BLOCK_BONUS:
      return std::make_unique<EditorTilesubgroupMenu>("Block + Bonus");

    case EDITOR_TILESUBGROUP_MENU_POLE_SIGNS:
      return std::make_unique<EditorTilesubgroupMenu>("Pole + Signs");

    case EDITOR_TILESUBGROUP_MENU_LIQUID:
      return std::make_unique<EditorTilesubgroupMenu>("Liquid");

    case EDITOR_TILESUBGROUP_MENU_CASTLE:
      return std::make_unique<EditorTilesubgroupMenu>("Castle");

    case EDITOR_TILESUBGROUP_MENU_HALLOWEEN:
      return std::make_unique<EditorTilesubgroupMenu>("Halloween");

    case EDITOR_TILESUBGROUP_MENU_INDUSTRIAL:
      return std::make_unique<EditorTilesubgroupMenu>("Industrial");

    case EDITOR_TILESUBGROUP_MENU_UNISOLID_LIGHTMAP:
      return std::make_unique<EditorTilesubgroupMenu>("Unisolid + Lightmap");

    case EDITOR_TILESUBGROUP_MENU_MISCELLANEOUS:
      return std::make_unique<EditorTilesubgroupMenu>("Miscellaneous");

    case EDITOR_TILESUBGROUP_MENU_RETRO_TILES:
      return std::make_unique<EditorTilesubgroupMenu>("Retro Tiles");

    case EDITOR_OBJECTGROUP_MENU:
      return std::make_unique<EditorObjectgroupMenu>();

    case EDITOR_SECTORS_MENU:
      return std::make_unique<EditorSectorsMenu>();

    case EDITOR_SECTOR_MENU:
      return std::make_unique<EditorSectorMenu>();

    case EDITOR_LEVEL_MENU:
      return std::make_unique<EditorLevelMenu>();

    case EDITOR_CONVERTERS_MENU:
      return std::make_unique<EditorConvertersMenu>();

    case INTEGRATIONS_MENU:
      return std::make_unique<IntegrationsMenu>();

    case PARTICLE_EDITOR_MENU:
      return std::make_unique<ParticleEditorMenu>();

    case PARTICLE_EDITOR_SAVE_AS:
      throw std::runtime_error("Cannot instantiate ParticleEditorSaveAs dialog "
        "from MenuStorage::create() or MenuManager::set_menu(), "
        "as it needs to be bound to a callback. "
        "Please instantiate ParticleEditorSaveAs directly");
      //return std::make_unique<ParticleEditorSaveAs>();

    case PARTICLE_EDITOR_OPEN:
      return std::make_unique<ParticleEditorOpen>();

    case ASSET_MENU:
      return std::make_unique<WebAssetMenu>();

    case CUSTOM_MENU_MENU:
      return std::make_unique<CustomMenuMenu>();

    case MULTIPLAYER_MENU:
      return std::make_unique<MultiplayerMenu>();

    case MULTIPLAYER_PLAYERS_MENU:
      return std::make_unique<MultiplayerPlayersMenu>();

    case NO_MENU:
      return std::unique_ptr<Menu>();

    default:
      log_warning << "unknown MenuId provided" << std::endl;
      assert(false);
      return std::unique_ptr<Menu>();
  }
}

const char* MenuStorage::MenuIdNames[] = {
  "NO_MENU",
  "MAIN_MENU",
  "OPTIONS_MENU",
  "INGAME_OPTIONS_MENU",
  "PROFILE_MENU",
  "WORLDSET_MENU",
  "CONTRIB_MENU",
  "CONTRIB_WORLD_MENU",
  "ADDON_MENU",
  "LANGPACK_MENU",
  "LANGPACK_AUTO_UPDATE_MENU",
  "LANGUAGE_MENU",
  "KEYBOARD_MENU",
  "JOYSTICK_MENU",
  "VIDEO_SYSTEM_MENU",
  "WORLDMAP_MENU",
  "WORLDMAP_CHEAT_MENU",
  "WORLDMAP_LEVEL_SELECT_MENU",
  "GAME_MENU",
  "CHEAT_MENU",
  "DEBUG_MENU",
  "EDITOR_LEVELSET_SELECT_MENU",
  "EDITOR_NEW_LEVELSET_MENU",
  "EDITOR_LEVEL_SELECT_MENU",
  "EDITOR_MENU",
  "EDITOR_TILEGROUP_MENU",
  "EDITOR_TILESUBGROUP_MENU_SNOW",
  "EDITOR_TILESUBGROUP_MENU_SNOW_BACKGROUND",
  "EDITOR_TILESUBGROUP_MENU_CRYSTAL",
  "EDITOR_TILESUBGROUP_MENU_FOREST",
  "EDITOR_TILESUBGROUP_MENU_FOREST_BACKGROUND",
  "EDITOR_TILESUBGROUP_MENU_CORRUPTED_FOREST",
  "EDITOR_TILESUBGROUP_MENU_CORRUPTED_BACKGROUND",
  "EDITOR_TILESUBGROUP_MENU_JAGGED_ROCKS",
  "EDITOR_TILESUBGROUP_MENU_BLOCK_BONUS",
  "EDITOR_TILESUBGROUP_MENU_POLE_SIGNS",
  "EDITOR_TILESUBGROUP_MENU_LIQUID",
  "EDITOR_TILESUBGROUP_MENU_CASTLE",
  "EDITOR_TILESUBGROUP_MENU_HALLOWEEN",
  "EDITOR_TILESUBGROUP_MENU_INDUSTRIAL",
  "EDITOR_TILESUBGROUP_MENU_UNISOLID_LIGHTMAP",
  "EDITOR_TILESUBGROUP_MENU_MISCELLANEOUS",
  "EDITOR_TILESUBGROUP_MENU_RETRO_TILES",
  "EDITOR_OBJECTGROUP_MENU",
  "EDITOR_SECTORS_MENU",
  "EDITOR_SECTOR_MENU",
  "EDITOR_LEVEL_MENU",
  "EDITOR_CONVERTERS_MENU",
  "PARTICLE_EDITOR_MENU",
  "PARTICLE_EDITOR_SAVE_AS",
  "PARTICLE_EDITOR_OPEN",
  "INTEGRATIONS_MENU",
  "ASSET_MENU",
  "CUSTOM_MENU_MENU",
  "MULTIPLAYER_MENU",
  "MULTIPLAYER_PLAYERS_MENU"
};

int
MenuStorage::get_menu_id(std::string menu_name)
{
  for (size_t i = 0; i < sizeof(MenuIdNames)/sizeof(MenuIdNames[0]); ++i) {
    if (MenuIdNames[i] == menu_name) {
      return i;  
    }
  }
  return -1;
}

/* EOF */
