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
#include <config.h>

#include "sprite/sprite_manager.hpp"
#include "gui/menu.hpp"
#include "gui/button.hpp"
#include "resources.hpp"
#include "file_system.hpp"
#include "tile_manager.hpp"
#include "object/gameobjs.hpp"
#include "object/player.hpp"

SpriteManager* sprite_manager = 0;
TileManager* tile_manager = 0;

MouseCursor* mouse_cursor = 0;

Font* gold_text;
Font* blue_text;
Font* gray_text;
Font* white_text;
Font* white_small_text;
Font* white_big_text;				    
				    
/* Load graphics/sounds shared between all levels: */
void load_shared()
{
  /* Load GUI/menu images: */
  checkbox = new Surface("images/engine/menu/checkbox-unchecked.png");
  checkbox_checked = new Surface("images/engine/menu/checkbox-checked.png");
  back = new Surface("images/engine/menu/arrow-back.png");
  arrow_left = new Surface("images/engine/menu/arrow-left.png");
  arrow_right = new Surface("images/engine/menu/arrow-right.png");

  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor("images/engine/menu/mousecursor.png");
  MouseCursor::set_current(mouse_cursor);

  /* Load global images: */
  gold_text = new Font("images/engine/fonts/gold.png",
                       "images/engine/fonts/shadow.png", 16, 18);
  blue_text = new Font("images/engine/fonts/blue.png", 
                       "images/engine/fonts/shadow.png", 16, 18, 3);
  white_text = new Font("images/engine/fonts/white.png",
                        "images/engine/fonts/shadow.png", 16, 18);
  gray_text = new Font("images/engine/fonts/gray.png",
                       "images/engine/fonts/shadow.png", 16, 18);
  white_small_text = new Font("images/engine/fonts/white-small.png",
                              "images/engine/fonts/shadow-small.png", 8, 9, 1);
  white_big_text = new Font("images/engine/fonts/white-big.png",
                            "images/engine/fonts/shadow-big.png", 20, 22, 3);

  Menu::default_font = white_text;
  Menu::active_font = blue_text;
  Menu::deactive_font = gray_text;
  Menu::label_font = white_big_text;
  Menu::field_font = gold_text;
  
  Button::info_font = white_small_text;

  sprite_manager = new SpriteManager("images/sprites.strf");
  tile_manager = new TileManager("images/tiles.strf");

  /* Tuxes: */
  char img_name[1024];
  for (int i = 0; i < GROWING_FRAMES; i++)
    {
      sprintf(img_name, "images/creatures/tux_grow/left-%i.png", i+1);
      growingtux_left[i] = new Surface(img_name);

      sprintf(img_name, "images/creatures/tux_grow/right-%i.png", i+1);
      growingtux_right[i] = new Surface(img_name);
    }

  small_tux = new TuxBodyParts();
  small_tux->head = 0;
  small_tux->body = sprite_manager->create("images/creatures/tux_small/small-tux-body.sprite");
  small_tux->arms = sprite_manager->create("images/creatures/tux_small/small-tux-arms.sprite");
  small_tux->feet = 0;

  big_tux = new TuxBodyParts();
  big_tux->head = sprite_manager->create("images/creatures/tux_big/big-tux-head.sprite");
  big_tux->body = sprite_manager->create("images/creatures/tux_big/big-tux-body.sprite");
  big_tux->arms = sprite_manager->create("images/creatures/tux_big/big-tux-arms.sprite");
  big_tux->feet = sprite_manager->create("images/creatures/tux_big/big-tux-feet.sprite");

  fire_tux = new TuxBodyParts();
  fire_tux->head = sprite_manager->create("images/creatures/tux_big/big-fire-tux-head.sprite");
  fire_tux->body = sprite_manager->create("images/creatures/tux_big/big-tux-body.sprite");
  fire_tux->arms = sprite_manager->create("images/creatures/tux_big/big-tux-arms.sprite");
  fire_tux->feet = sprite_manager->create("images/creatures/tux_big/big-tux-feet.sprite");

  ice_tux = new TuxBodyParts();
  ice_tux->head = sprite_manager->create("images/creatures/tux_big/big-tux-head.sprite");
  ice_tux->body = sprite_manager->create("images/creatures/tux_big/big-tux-body.sprite");
  ice_tux->arms = sprite_manager->create("images/creatures/tux_big/big-tux-arms.sprite");
  ice_tux->feet = sprite_manager->create("images/creatures/tux_big/big-tux-feet.sprite");

  /* Tux life: */
  tux_life = new Surface("images/creatures/tux_small/tux-life.png");
  
  player_status = new PlayerStatus();
}

/* Free shared data: */
void unload_shared()
{
  /* Free global images: */
  delete gold_text;
  delete white_text;
  delete blue_text;
  delete gray_text;
  delete white_small_text;
  delete white_big_text;
  
  delete tux_life;

  delete small_tux;
  delete big_tux;
  delete fire_tux;
  delete ice_tux;

  for (int i = 0; i < GROWING_FRAMES; i++) {
    delete growingtux_left[i];
    delete growingtux_right[i];
  }

  delete sprite_manager;
  sprite_manager = 0;
  delete tile_manager;
  tile_manager = 0;

  /* Free GUI/menu images: */
  delete checkbox;
  delete checkbox_checked;
  delete back;
  delete arrow_left;
  delete arrow_right;

  /* Free mouse-cursor */
  delete mouse_cursor;
}

