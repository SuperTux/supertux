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

#include <string>
#include "audio/sound.h"

/*global variable*/
bool use_sound = true;    /* handle sound on/off menu and command-line option */
bool use_music = true;    /* handle music on/off menu and command-line option */
bool audio_device = true; /* != 0: available and initialized */

#include <SDL_mixer.h>

std::vector<Mix_Chunk*> sounds;

/* --- OPEN THE AUDIO DEVICE --- */

int open_audio (int frequency, Uint16 format, int channels, int chunksize)
{
  if (Mix_OpenAudio( frequency, format, channels, chunksize ) < 0)
    return -1;

  // allocate 16 channels for mixing
  if (Mix_AllocateChannels(8)  != 8)
    return -2;
  
  return 0;
}


/* --- CLOSE THE AUDIO DEVICE --- */

void close_audio( void )
{
  if (audio_device) {
    Mix_CloseAudio();
  }
}


/* --- LOAD A SOUND --- */

Mix_Chunk* load_sound(const std::string& file)
{
  if(!audio_device)
    return 0;
  
  Mix_Chunk* snd = Mix_LoadWAV(file.c_str());

  /*if (snd == 0)
    st_abort("Can't load", file);*/

  return(snd);
}

void free_chunk(Mix_Chunk *chunk)
{
  Mix_FreeChunk( chunk );
}

