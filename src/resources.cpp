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

#include "app/globals.h"
#include "special/sprite_manager.h"
#include "app/setup.h"
#include "gui/menu.h"
#include "gui/button.h"
#include "scene.h"
#include "resources.h"
#include "tile_manager.h"
#include "object/gameobjs.h"
#include "object/player.h"

Menu* main_menu      = 0;
Menu* game_menu      = 0;
Menu* options_menu   = 0;
Menu* options_keys_menu     = 0;
Menu* options_joystick_menu = 0;
Menu* highscore_menu = 0;
Menu* load_game_menu = 0;
Menu* save_game_menu = 0;
Menu* contrib_menu   = 0;
Menu* contrib_subset_menu   = 0;

MusicRef herring_song;
MusicRef level_end_song;

SpriteManager* sprite_manager = 0;
TileManager* tile_manager = 0;

char * soundfilenames[NUM_SOUNDS] = {
                                       "/sounds/jump.wav",
                                       "/sounds/bigjump.wav",
                                       "/sounds/skid.wav",
                                       "/sounds/coin.wav",
                                       "/sounds/invincible.wav",
                                       "/sounds/brick.wav",
                                       "/sounds/hurt.wav",
                                       "/sounds/squish.wav",
                                       "/sounds/fall.wav",
                                       "/sounds/ricochet.wav",
                                       "/sounds/bump-upgrade.wav",
                                       "/sounds/upgrade.wav",
                                       "/sounds/grow.wav",
                                       "/sounds/fire-flower.wav",
                                       "/sounds/shoot.wav",
                                       "/sounds/lifeup.wav",
                                       "/sounds/stomp.wav",
                                       "/sounds/kick.wav",
                                       "/sounds/explosion.wav",
                                       "/sounds/warp.wav",
                                       "/sounds/fireworks.wav"
                                    };


Font* gold_text;
Font* blue_text;
Font* gray_text;
Font* yellow_nums;
Font* white_text;
Font* white_small_text;
Font* white_big_text;				    
				    
/* Load graphics/sounds shared between all levels: */
void loadshared()
{
  /* Load global images: */
  gold_text = new Font(datadir + "/images/fonts/gold.png", Font::TEXT, 16,18);
  blue_text = new Font(datadir + "/images/fonts/blue.png", Font::TEXT, 16,18,3);
  white_text  = new Font(datadir + "/images/fonts/white.png",
                         Font::TEXT, 16,18);
  gray_text  = new Font(datadir + "/images/fonts/gray.png",
                        Font::TEXT, 16,18);
  white_small_text = new Font(datadir + "/images/fonts/white-small.png",
                              Font::TEXT, 8,9, 1);
  white_big_text   = new Font(datadir + "/images/fonts/white-big.png",
                              Font::TEXT, 20,22, 3);
  yellow_nums = new Font(datadir + "/images/fonts/numbers.png",
                         Font::NUM, 32,32);

  Menu::default_font = white_text;
  Menu::active_font = blue_text;
  Menu::deactive_font = gray_text;
  Menu::label_font = white_big_text;
  Menu::field_font = gold_text;
  
  Button::info_font = white_small_text;

  int i;

  sprite_manager = new SpriteManager(
      get_resource_filename("/images/supertux.strf"));
  tile_manager = new TileManager("/images/tilesets/supertux.stgt");

  /* Tuxes: */
  char img_name[1024];
  for (int i = 0; i < GROWING_FRAMES; i++)
    {
      sprintf(img_name, "%s/images/shared/tux-grow-left-%i.png", datadir.c_str(), i+1);
      growingtux_left[i] = new Surface(img_name, true);

      sprintf(img_name, "%s/images/shared/tux-grow-right-%i.png", datadir.c_str(), i+1);
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
  tux_life = new Surface(datadir + "/images/shared/tux-life.png",
                         true);

  /* Sound effects: */

  for (i = 0; i < NUM_SOUNDS; i++)
    SoundManager::get()->add_sound(SoundManager::get
                      ()->load_sound(datadir + soundfilenames[i]),i);

  /* Herring song */
  herring_song = SoundManager::get
                   ()->load_music(datadir + "/music/salcon.mod");
  level_end_song = SoundManager::get
                     ()->load_music(datadir + "/music/leveldone.mod");
}

/* Free shared data: */
void unloadshared(void)
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
}

std::string get_resource_filename(const std::string& resource)
{
  std::string filepath = st_dir + resource;
  if(FileSystem::faccessible(filepath))
    return filepath;
  
  filepath = datadir + resource;
  if(FileSystem::faccessible(filepath))
    return filepath;

  std::cerr << "Couldn't find resource: '" << resource  << "'." << std::endl;
  return "";
}
