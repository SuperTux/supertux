//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Duong-Khang NGUYEN <neoneurone@users.sf.net>
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

#ifndef SUPERTUX_SOUND_H
#define SUPERTUX_SOUND_H

//#include "defines.h"     /* get YES/NO defines */
#include <vector>

/*global variable*/
extern bool use_sound;           /* handle sound on/off menu and command-line option */
extern bool use_music;           /* handle music on/off menu and command-line */
extern bool audio_device;        /* != 0: available and initialized */

/* enum of different internal music types */
enum Music_Type {
  NO_MUSIC,
  LEVEL_MUSIC,
  HURRYUP_MUSIC,
  HERRING_MUSIC
};

#include <string>
#include <SDL_mixer.h>

/* variables for stocking the sound and music */
extern std::vector<Mix_Chunk*> sounds;

/* functions handling the sound and music */
int open_audio(int frequency, Uint16 format, int channels, int chunksize);
void close_audio( void );

Mix_Chunk * load_sound(const std::string& file);
void free_chunk(Mix_Chunk*chunk);

#endif /*SUPERTUX_SOUND_H*/
