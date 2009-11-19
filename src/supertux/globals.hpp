//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_GLOBALS_HPP
#define HEADER_SUPERTUX_SUPERTUX_GLOBALS_HPP

typedef struct SDL_Surface SDL_Surface;
namespace tinygettext { class DictionaryManager; }
class Config;
class JoystickKeyboardController;
class PlayerStatus;
class ScreenManager;
class SoundManager;
class SpriteManager;
class TextureManager;
class TileManager;
class TileSet;

/** The width of the display (this is a logical value, not the
    physical value, since aspect_ration and projection_area might
    shrink or scale things) */
extern int SCREEN_WIDTH;

/** The width of the display (this is a logical value, not the
    physical value, since aspect_ration and projection_area might
    shrink or scale things) */
extern int SCREEN_HEIGHT;

// global variables
extern JoystickKeyboardController* g_main_controller;

extern SDL_Surface* g_screen;

extern ScreenManager* g_screen_manager;

extern TextureManager* texture_manager;

extern SoundManager* sound_manager;

extern Config* g_config;

extern tinygettext::DictionaryManager* dictionary_manager;

extern float game_time;
extern float real_time;

extern TileManager *tile_manager;

/** this is only set while loading a map */
extern TileSet     *current_tileset;

// global player state
extern PlayerStatus* player_status;

extern SpriteManager* sprite_manager;

extern float g_game_speed;

#endif

/* EOF */
