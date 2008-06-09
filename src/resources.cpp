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
#include <config.h>

#include "sprite/sprite_manager.hpp"
#include "gui/menu.hpp"
#include "gui/button.hpp"
#include "resources.hpp"
#include "file_system.hpp"
#include "tile_manager.hpp"
#include "object/gameobjs.hpp"
#include "object/player.hpp"

MouseCursor* mouse_cursor = NULL;

Font* gold_text = NULL;
Font* gold_fixed_text = NULL;
Font* blue_text = NULL;
Font* gray_text = NULL;
Font* white_text = NULL;
Font* white_small_text = NULL;
Font* white_big_text = NULL;

/* Load graphics/sounds shared between all levels: */
void load_shared()
{
  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor("images/engine/menu/mousecursor.png");
  MouseCursor::set_current(mouse_cursor);

  /* Load global images: */
  gold_text  = new Font(Font::VARIABLE,
                        "images/engine/fonts/gold.png",
                        "images/engine/fonts/shadow.png", 16, 18);
  gold_fixed_text  = new Font(Font::FIXED,
                        "images/engine/fonts/gold.png",
                        "images/engine/fonts/shadow.png", 16, 18);
  blue_text  = new Font(Font::VARIABLE,
                        "images/engine/fonts/blue.png",
                        "images/engine/fonts/shadow.png", 16, 18, 3);
  white_text = new Font(Font::VARIABLE,
                        "images/engine/fonts/white.png",
                        "images/engine/fonts/shadow.png", 16, 18);
  gray_text  = new Font(Font::VARIABLE,
                        "images/engine/fonts/gray.png",
                       "images/engine/fonts/shadow.png", 16, 18);
  white_small_text = new Font(Font::VARIABLE,
                              "images/engine/fonts/white-small.png",
                              "images/engine/fonts/shadow-small.png", 8, 9, 1);
  white_big_text = new Font(Font::VARIABLE,
                            "images/engine/fonts/white-big.png",
                            "images/engine/fonts/shadow-big.png", 20, 22, 3);

  Menu::default_font  = white_text;
  Menu::active_font   = blue_text;
  Menu::inactive_font = gray_text;
  Menu::label_font    = white_big_text;
  Menu::field_font    = gold_text;

  Button::info_font = white_small_text;

  tile_manager   = new TileManager();
  sprite_manager = new SpriteManager();

  player_status = new PlayerStatus();
}

/* Free shared data: */
void unload_shared()
{
  /* Free global images: */
  delete gold_text;
  delete gold_fixed_text;
  delete white_text;
  delete blue_text;
  delete gray_text;
  delete white_small_text;
  delete white_big_text;

  delete sprite_manager;
  sprite_manager = NULL;

  /* Free mouse-cursor */
  delete mouse_cursor;

  delete player_status;
  player_status = NULL;
}
