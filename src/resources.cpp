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

#include "sprite/sprite_manager.h"
#include "gui/menu.h"
#include "gui/button.h"
#include "resources.h"
#include "file_system.h"
#include "tile_manager.h"
#include "object/gameobjs.h"
#include "object/player.h"

SpriteManager* sprite_manager = 0;
TileManager* tile_manager = 0;
SoundManager* sound_manager = 0;

MouseCursor* mouse_cursor = 0;

Font* gold_text;
Font* blue_text;
Font* gray_text;
Font* yellow_nums;
Font* white_text;
Font* white_small_text;
Font* white_big_text;				    
				    
/* Load graphics/sounds shared between all levels: */
void load_shared()
{
  /* Load GUI/menu images: */
  checkbox = new Surface("images/engine/menu/checkbox-unchecked.png", true);
  checkbox_checked = new Surface("images/engine/menu/checkbox-checked.png", true);
  back = new Surface("images/engine/menu/arrow-back.png", true);
  arrow_left = new Surface("images/engine/menu/arrow-left.png", true);
  arrow_right = new Surface("images/engine/menu/arrow-right.png", true);

  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor("images/engine/menu/mousecursor.png");
  MouseCursor::set_current(mouse_cursor);

  /* Load global images: */
  gold_text = new Font("images/engine/fonts/gold.png", Font::TEXT, 16,18);
  blue_text = new Font("images/engine/fonts/blue.png", Font::TEXT, 16,18,3);
  white_text  = new Font("images/engine/fonts/white.png", Font::TEXT, 16,18);
  gray_text  = new Font("images/engine/fonts/gray.png", Font::TEXT, 16,18);
  white_small_text = new Font("images/engine/fonts/white-small.png",
                              Font::TEXT, 8,9, 1);
  white_big_text   = new Font("images/engine/fonts/white-big.png",
                              Font::TEXT, 20,22, 3);
  yellow_nums = new Font("images/engine/fonts/numbers.png", Font::NUM, 32,32);

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
      growingtux_left[i] = new Surface(img_name, true);

      sprintf(img_name, "images/creatures/tux_grow/right-%i.png", i+1);
      growingtux_right[i] = new Surface(img_name, true);
    }

  small_tux = new TuxBodyParts();
  small_tux->head = 0;
  small_tux->body = sprite_manager->create("small-tux-body");
  small_tux->arms = sprite_manager->create("small-tux-arms");
  small_tux->feet = 0;

  big_tux = new TuxBodyParts();
  big_tux->head = sprite_manager->create("big-tux-head");
  big_tux->body = sprite_manager->create("big-tux-body");
  big_tux->arms = sprite_manager->create("big-tux-arms");
  big_tux->feet = sprite_manager->create("big-tux-feet");

  fire_tux = new TuxBodyParts();
  fire_tux->head = sprite_manager->create("big-fire-tux-head");
  fire_tux->body = sprite_manager->create("big-tux-body");
  fire_tux->arms = sprite_manager->create("big-tux-arms");
  fire_tux->feet = sprite_manager->create("big-tux-feet");

  ice_tux = new TuxBodyParts();
  ice_tux->head = sprite_manager->create("big-tux-head");
  ice_tux->body = sprite_manager->create("big-tux-body");
  ice_tux->arms = sprite_manager->create("big-tux-arms");
  ice_tux->feet = sprite_manager->create("big-tux-feet");

  /* Objects */
  load_object_gfx();

  /* Tux life: */
  tux_life = new Surface("images/creatures/tux_small/tux-life.png", true);

#if 0
  /* Sound effects: */
  sound_manager->preload_sound("jump");
  sound_manager->preload_sound("bigjump");
  sound_manager->preload_sound("skid");
  sound_manager->preload_sound("coin");
  sound_manager->preload_sound("invincible");
  sound_manager->preload_sound("brick");
  sound_manager->preload_sound("hurt");
  sound_manager->preload_sound("squish");
  sound_manager->preload_sound("fall");
  sound_manager->preload_sound("ricochet");
  sound_manager->preload_sound("bump-upgrade");
  sound_manager->preload_sound("upgrade");
  sound_manager->preload_sound("grow");
  sound_manager->preload_sound("fire-flower");
  sound_manager->preload_sound("shoot");
  sound_manager->preload_sound("lifeup");
  sound_manager->preload_sound("stomp");
  sound_manager->preload_sound("kick");
  sound_manager->preload_sound("explosion");
  sound_manager->preload_sound("warp");
  sound_manager->preload_sound("fireworks");
#endif
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
  delete yellow_nums;
  
  free_object_gfx();

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

