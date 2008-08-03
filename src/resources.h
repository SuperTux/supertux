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

#ifndef NOSOUND
#include "musicref.h"
#endif

class SpriteManager;
#ifndef NOSOUND
class MusicManager;
#endif

extern Surface* img_waves[3]; 
extern Surface* img_water;
extern Surface* img_pole;
extern Surface* img_poletop;
extern Surface* img_flag[2];
extern Surface* img_cloud[2][4];

extern Surface* img_super_bkgd;

#ifndef NOSOUND
extern MusicRef herring_song;
extern MusicRef level_end_song;
extern MusicManager* music_manager;
#endif

extern SpriteManager* sprite_manager;

void loadshared();
void unloadshared();

#endif

/* EOF */

