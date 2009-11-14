//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#include <config.h>

#include "supertux/resources.hpp"

#include "gui/mousecursor.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/player_status.hpp"
#include "video/font.hpp"

MouseCursor* mouse_cursor = NULL;

Font* fixed_font = NULL;
Font* normal_font = NULL;
Font* small_font = NULL;
Font* big_font = NULL;

/* Load graphics/sounds shared between all levels: */
void load_shared()
{
  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor("images/engine/menu/mousecursor.png");
  MouseCursor::set_current(mouse_cursor);

  /* Load global images: */
  fixed_font = new Font(Font::FIXED, "fonts/white.stf");
  normal_font = new Font(Font::VARIABLE, "fonts/white.stf");
  small_font = new Font(Font::VARIABLE, "fonts/white-small.stf", 1);
  big_font = new Font(Font::VARIABLE, "fonts/white-big.stf", 3);

  tile_manager   = new TileManager();
  sprite_manager = new SpriteManager();

  player_status = new PlayerStatus();
}

/* Free shared data: */
void unload_shared()
{
  /* Free global images: */
  delete normal_font;
  delete small_font;
  delete big_font;

  delete sprite_manager;
  sprite_manager = NULL;

  /* Free mouse-cursor */
  delete mouse_cursor;

  delete player_status;
  player_status = NULL;
}
