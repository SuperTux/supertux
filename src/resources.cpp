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

#include "globals.h"
#include "scene.h"
#include "player.h"
#include "badguy.h"
#include "gameobjs.h"
#include "special.h"
#include "resources.h"
#include "sprite_manager.h"
#include "setup.h"

Surface* img_waves[3]; 
Surface* img_water;
Surface* img_pole;
Surface* img_poletop;
Surface* img_flag[2];
Surface* img_cloud[2][4];

Surface* img_red_glow;

MusicRef herring_song;

SpriteManager* sprite_manager = 0;
MusicManager* music_manager = 0;

/* Load graphics/sounds shared between all levels: */
void loadshared()
{
  int i;

  sprite_manager = new SpriteManager(datadir + "/supertux.strf");
  music_manager = new MusicManager();
  music_manager->enable_music(use_music);

  /* Tuxes: */
  smalltux_star = sprite_manager->load("smalltux-star");
  largetux_star = sprite_manager->load("largetux-star");
  smalltux_gameover = sprite_manager->load("smalltux-gameover");

  smalltux_stand_left  = sprite_manager->load("smalltux-stand-left");
  smalltux_stand_right = sprite_manager->load("smalltux-stand-right");
  smalltux_walk_left   = sprite_manager->load("smalltux-walk-left");
  smalltux_walk_right  = sprite_manager->load("smalltux-walk-right");
  smalltux_jump_left   = sprite_manager->load("smalltux-jump-left");
  smalltux_jump_right  = sprite_manager->load("smalltux-jump-right");
  smalltux_kick_left   = sprite_manager->load("smalltux-kick-left");
  smalltux_kick_right  = sprite_manager->load("smalltux-kick-right");
  smalltux_skid_left   = sprite_manager->load("smalltux-skid-left");
  smalltux_skid_right  = sprite_manager->load("smalltux-skid-right");
  smalltux_grab_left   = sprite_manager->load("smalltux-grab-left");
  smalltux_grab_right  = sprite_manager->load("smalltux-grab-right");

  largetux_stand_left  = sprite_manager->load("largetux-stand-left");
  largetux_stand_right = sprite_manager->load("largetux-stand-right");
  largetux_walk_left   = sprite_manager->load("largetux-walk-left");
  largetux_walk_right  = sprite_manager->load("largetux-walk-right");
  largetux_jump_left   = sprite_manager->load("largetux-jump-left");
  largetux_jump_right  = sprite_manager->load("largetux-jump-right");
  largetux_kick_left   = sprite_manager->load("largetux-kick-left");
  largetux_kick_right  = sprite_manager->load("largetux-kick-right");
  largetux_skid_right  = sprite_manager->load("largetux-skid-right");
  largetux_skid_left   = sprite_manager->load("largetux-skid-left");
  largetux_grab_left   = sprite_manager->load("largetux-grab-left");
  largetux_grab_right  = sprite_manager->load("largetux-grab-right");
  largetux_duck_left   = sprite_manager->load("largetux-duck-left");
  largetux_duck_right  = sprite_manager->load("largetux-duck-right");

  duckfiretux_right = new Surface(datadir +
               "/images/shared/duckfiretux-right.png",
               USE_ALPHA);

  duckfiretux_left = new Surface(datadir +
               "/images/shared/duckfiretux-left.png",
               USE_ALPHA);

  firetux_right[0] = new Surface(datadir + "/images/shared/firetux-right-0.png", USE_ALPHA);
  firetux_right[1] = new Surface(datadir + "/images/shared/firetux-right-1.png", USE_ALPHA);
  firetux_right[2] = new Surface(datadir + "/images/shared/firetux-right-2.png", USE_ALPHA);

  firetux_left[0] = new Surface(datadir + "/images/shared/firetux-left-0.png", USE_ALPHA);
  firetux_left[1] = new Surface(datadir + "/images/shared/firetux-left-1.png", USE_ALPHA);
  firetux_left[2] = new Surface(datadir + "/images/shared/firetux-left-2.png", USE_ALPHA);

  bigfiretux_right[0] = new Surface(datadir + "/images/shared/bigfiretux-right-0.png",
               USE_ALPHA);

  bigfiretux_right[1] = new Surface(datadir + "/images/shared/bigfiretux-right-1.png",
               USE_ALPHA);

  bigfiretux_right[2] = new Surface(datadir + "/images/shared/bigfiretux-right-2.png",
               USE_ALPHA);

  bigfiretux_right_jump = new Surface(datadir + "/images/shared/bigfiretux-right-jump.png", USE_ALPHA);

  bigfiretux_left[0] = new Surface(datadir + "/images/shared/bigfiretux-left-0.png",
               USE_ALPHA);

  bigfiretux_left[1] = new Surface(datadir + "/images/shared/bigfiretux-left-1.png",
               USE_ALPHA);

  bigfiretux_left[2] = new Surface(datadir + "/images/shared/bigfiretux-left-2.png",
               USE_ALPHA);

  bigfiretux_left_jump = new Surface(datadir + "/images/shared/bigfiretux-left-jump.png", USE_ALPHA);

  skidfiretux_right = new Surface(datadir +
               "/images/shared/skidfiretux-right.png",
               USE_ALPHA);

  skidfiretux_left = new Surface(datadir +
               "/images/shared/skidfiretux-left.png",
               USE_ALPHA);


  /* Water: */


  img_water = new Surface(datadir + "/images/shared/water.png", IGNORE_ALPHA);

  img_waves[0] = new Surface(datadir + "/images/shared/waves-0.png",
               USE_ALPHA);

  img_waves[1] = new Surface(datadir + "/images/shared/waves-1.png",
               USE_ALPHA);

  img_waves[2] = new Surface(datadir + "/images/shared/waves-2.png",
               USE_ALPHA);


  /* Pole: */

  img_pole = new Surface(datadir + "/images/shared/pole.png", USE_ALPHA);
  img_poletop = new Surface(datadir + "/images/shared/poletop.png",
               USE_ALPHA);


  /* Flag: */

  img_flag[0] = new Surface(datadir + "/images/shared/flag-0.png",
               USE_ALPHA);
  img_flag[1] = new Surface(datadir + "/images/shared/flag-1.png",
               USE_ALPHA);


  /* Cloud: */

  img_cloud[0][0] = new Surface(datadir + "/images/shared/cloud-00.png",
               USE_ALPHA);

  img_cloud[0][1] = new Surface(datadir + "/images/shared/cloud-01.png",
               USE_ALPHA);

  img_cloud[0][2] = new Surface(datadir + "/images/shared/cloud-02.png",
               USE_ALPHA);

  img_cloud[0][3] = new Surface(datadir + "/images/shared/cloud-03.png",
               USE_ALPHA);


  img_cloud[1][0] = new Surface(datadir + "/images/shared/cloud-10.png",
               USE_ALPHA);

  img_cloud[1][1] = new Surface(datadir + "/images/shared/cloud-11.png",
               USE_ALPHA);

  img_cloud[1][2] = new Surface(datadir + "/images/shared/cloud-12.png",
               USE_ALPHA);

  img_cloud[1][3] = new Surface(datadir + "/images/shared/cloud-13.png",
               USE_ALPHA);


  /* Bad guys: */
  load_badguy_gfx();

  /* Upgrades: */
  load_special_gfx();

  /* Weapons: */
  img_red_glow = new Surface(datadir + "/images/shared/red-glow.png",
               USE_ALPHA);

  /* Distros: */
  img_distro[0] = new Surface(datadir + "/images/tilesets/coin1.png",
               USE_ALPHA);

  img_distro[1] = new Surface(datadir + "/images/tilesets/coin2.png",
               USE_ALPHA);

  img_distro[2] = new Surface(datadir + "/images/tilesets/coin3.png",
               USE_ALPHA);

  img_distro[3] = new Surface(datadir + "/images/tilesets/coin2.png",
               USE_ALPHA);


  /* Tux life: */

  tux_life = new Surface(datadir + "/images/shared/tux-life.png",
                         USE_ALPHA);

  /* Sound effects: */

  /* if (use_sound) // this will introduce SERIOUS bugs here ! because "load_sound"
                    // initialize sounds[i] with the correct pointer's value:
                    // NULL or something else. And it will be dangerous to
                    // play with not-initialized pointers.
                    // This is also true with if (use_music)
                    Send a mail to me: neoneurone@users.sf.net, if you have another opinion. :)
  */
  for (i = 0; i < NUM_SOUNDS; i++)
    sounds[i] = load_sound(datadir + soundfilenames[i]);

  /* Herring song */
  herring_song = music_manager->load_music(datadir + "/music/SALCON.MOD");
}


/* Free shared data: */
void unloadshared(void)
{
  int i;

  free_special_gfx();
  free_badguy_gfx();

  delete smalltux_gameover;

  delete img_water;
  for (i = 0; i < 3; i++)
    delete img_waves[i];

  delete img_pole;
  delete img_poletop;

  for (i = 0; i < 2; i++)
    delete img_flag[i];

  for (i = 0; i < 4; i++)
    {
      delete img_distro[i];
      delete img_cloud[0][i];
      delete img_cloud[1][i];
    }

  for (i = 0; i < NUM_SOUNDS; i++)
    free_chunk(sounds[i]);

  delete sprite_manager;
  sprite_manager = 0;
  delete music_manager;
  music_manager = 0;
}

/* EOF */
