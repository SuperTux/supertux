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
#ifndef NOSOUND
#include "sound.h"
#endif
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

#ifndef NOSOUND
MusicRef herring_song;
MusicRef level_end_song;
MusicManager* music_manager = 0;
#endif

SpriteManager* sprite_manager = 0;

/* Load graphics/sounds shared between all levels: */
void loadshared()
{
  int i;

  sprite_manager = new SpriteManager(datadir + "/supertux.strf");
#ifndef NOSOUND
  music_manager = new MusicManager();
  music_manager->enable_music(use_music);
#endif

  /* Tuxes: */
  smalltux_star = sprite_manager->load("smalltux-star");
  largetux_star = sprite_manager->load("largetux-star");
  smalltux_gameover = sprite_manager->load("smalltux-gameover");

  smalltux.stand_left  = sprite_manager->load("smalltux-stand-left");
  smalltux.stand_right = sprite_manager->load("smalltux-stand-right");
  smalltux.walk_left   = sprite_manager->load("smalltux-walk-left");
  smalltux.walk_right  = sprite_manager->load("smalltux-walk-right");
  smalltux.jump_left   = sprite_manager->load("smalltux-jump-left");
  smalltux.jump_right  = sprite_manager->load("smalltux-jump-right");
  smalltux.kick_left   = sprite_manager->load("smalltux-kick-left");
  smalltux.kick_right  = sprite_manager->load("smalltux-kick-right");
  smalltux.skid_left   = sprite_manager->load("smalltux-skid-left");
  smalltux.skid_right  = sprite_manager->load("smalltux-skid-right");
  smalltux.grab_left   = sprite_manager->load("smalltux-grab-left");
  smalltux.grab_right  = sprite_manager->load("smalltux-grab-right");

  largetux.stand_left  = sprite_manager->load("largetux-stand-left");
  largetux.stand_right = sprite_manager->load("largetux-stand-right");
  largetux.walk_left   = sprite_manager->load("largetux-walk-left");
  largetux.walk_right  = sprite_manager->load("largetux-walk-right");
  largetux.jump_left   = sprite_manager->load("largetux-jump-left");
  largetux.jump_right  = sprite_manager->load("largetux-jump-right");
  largetux.kick_left   = sprite_manager->load("largetux-kick-left");
  largetux.kick_right  = sprite_manager->load("largetux-kick-right");
  largetux.skid_right  = sprite_manager->load("largetux-skid-right");
  largetux.skid_left   = sprite_manager->load("largetux-skid-left");
  largetux.grab_left   = sprite_manager->load("largetux-grab-left");
  largetux.grab_right  = sprite_manager->load("largetux-grab-right");
  largetux.duck_left   = sprite_manager->load("largetux-duck-left");
  largetux.duck_right  = sprite_manager->load("largetux-duck-right");

  firetux.stand_left  = sprite_manager->load("firetux-stand-left");
  firetux.stand_right = sprite_manager->load("firetux-stand-right");
  firetux.walk_left   = sprite_manager->load("firetux-walk-left");
  firetux.walk_right  = sprite_manager->load("firetux-walk-right");
  firetux.jump_left   = sprite_manager->load("firetux-jump-left");
  firetux.jump_right  = sprite_manager->load("firetux-jump-right");
  firetux.kick_left   = sprite_manager->load("firetux-kick-left");
  firetux.kick_right  = sprite_manager->load("firetux-kick-right");
  firetux.skid_right  = sprite_manager->load("firetux-skid-right");
  firetux.skid_left   = sprite_manager->load("firetux-skid-left");
  firetux.grab_left   = sprite_manager->load("firetux-grab-left");
  firetux.grab_right  = sprite_manager->load("firetux-grab-right");
  firetux.duck_left   = sprite_manager->load("firetux-duck-left");
  firetux.duck_right  = sprite_manager->load("firetux-duck-right");

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
#ifndef NOSOUND
#ifndef GP2X
  for (i = 0; i < NUM_SOUNDS; i++)
    sounds[i] = load_sound(datadir + soundfilenames[i]);
#endif

  /* Herring song */
  herring_song = music_manager->load_music(datadir + "/music/salcon.mod");
  level_end_song = music_manager->load_music(datadir + "/music/leveldone.mod");
#endif
}


/* Free shared data: */
void unloadshared(void)
{
  int i;

  free_special_gfx();
  free_badguy_gfx();

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

  delete tux_life;

#ifndef NOSOUND
#ifndef GP2X
  for (i = 0; i < NUM_SOUNDS; i++)
    free_chunk(sounds[i]);
#endif
#endif

  delete sprite_manager;
  sprite_manager = 0;
#ifndef NOSOUND
  delete music_manager;
  music_manager = 0;
#endif
}

/* EOF */
