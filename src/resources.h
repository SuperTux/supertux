//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef SUPERTUX_RESOURCES_H
#define SUPERTUX_RESOURCES_H

#include "audio/musicref.h"

using namespace SuperTux;

class SpriteManager;
class Menu;
class Font;
class Surface;
class SoundManager;
class TileManager;

extern Surface* img_super_bkgd;
extern Surface* tux_life;

extern MusicRef herring_song;
extern MusicRef level_end_song;

extern SpriteManager* sprite_manager;
extern TileManager* tile_manager;
extern SoundManager* sound_manager;

extern Menu* contrib_menu;
extern Menu* contrib_subset_menu;
extern Menu* main_menu;
extern Menu* game_menu;
extern Menu* options_menu;
extern Menu* load_game_menu;

extern Font* gold_text;
extern Font* white_text;
extern Font* blue_text;
extern Font* gray_text;
extern Font* white_small_text;
extern Font* white_big_text;
extern Font* yellow_nums;

/** maps a virtual resource path to a real path (ie. levels/bla is mapped to
 * $DATADIR/levels/bla or $HOME/.supertux/levels/bla)
 * All paths inside the game should be handled in as virtual paths and then
 * expanded with this function just before the call to fopen or std::ifstream.
 */
std::string get_resource_filename(const std::string& resource);

void load_shared();
void unload_shared();

#endif
