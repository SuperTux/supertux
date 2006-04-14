//  $Id$
//
//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

class SpriteManager;
class Menu;
class Font;
class Surface;
class SoundManager;
class TileManager;
class MouseCursor;

extern Menu* main_menu;
extern Menu* game_menu;

extern MouseCursor* mouse_cursor;

extern Font* gold_text;
extern Font* white_text;
extern Font* blue_text;
extern Font* gray_text;
extern Font* white_small_text;
extern Font* white_big_text;

void load_shared();
void unload_shared();

#endif
