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

#include "defines.h"
#include "globals.h"
#include "sound.h"
#include "setup.h"

/*global variable*/
bool use_sound;           /* handle sound on/off menu and command-line option */
bool use_music;           /* handle music on/off menu and command-line option */
bool audio_device;        /* != 0: available and initialized */
int current_music;

char * soundfilenames[NUM_SOUNDS] = {
                                       "/sounds/jump.wav",
                                       "/sounds/bigjump.wav",
                                       "/sounds/skid.wav",
                                       "/sounds/distro.wav",
                                       "/sounds/herring.wav",
                                       "/sounds/brick.wav",
                                       "/sounds/hurt.wav",
                                       "/sounds/squish.wav",
                                       "/sounds/fall.wav",
                                       "/sounds/ricochet.wav",
                                       "/sounds/bump-upgrade.wav",
                                       "/sounds/upgrade.wav",
                                       "/sounds/excellent.wav",
                                       "/sounds/coffee.wav",
                                       "/sounds/shoot.wav",
                                       "/sounds/lifeup.wav",
                                       "/sounds/stomp.wav",
                                       "/sounds/kick.wav"
                                    };


#include <SDL_mixer.h>

Mix_Chunk * sounds[NUM_SOUNDS];
Mix_Music * herring_song = 0;
Mix_Music * current_song = 0;

/* --- OPEN THE AUDIO DEVICE --- */

int open_audio (int frequency, Uint16 format, int channels, int chunksize)
{
  /* if success we reserved some channels and register panning effects */
  if (Mix_OpenAudio( frequency, format, channels, chunksize ) == 0)
    {
      if (Mix_ReserveChannels( SOUND_RESERVED_CHANNELS )
                            != SOUND_RESERVED_CHANNELS )
        {
          DEBUG_MSG( "Warning: open_audio could'nt reserve channels" );
        }

      /* prepare the spanning effects, no error checking */
      Mix_SetPanning( SOUND_LEFT_SPEAKER, 230, 24 );
      Mix_SetPanning( SOUND_RIGHT_SPEAKER, 24, 230 );
      return 0;
    }
  else
    {
      return -1;
    }
}


/* --- CLOSE THE AUDIO DEVICE --- */

void close_audio( void )
{
  if (audio_device) {
    Mix_UnregisterAllEffects( SOUND_LEFT_SPEAKER );
    Mix_UnregisterAllEffects( SOUND_RIGHT_SPEAKER );
    Mix_CloseAudio();
  }
}


/* --- LOAD A SOUND --- */

Mix_Chunk * load_sound(const std::string& file)
{
  Mix_Chunk * snd;

  snd = Mix_LoadWAV(file.c_str());

  /* printf message and abort if there is an initialized audio device */
  if ((snd == NULL) && audio_device)
    st_abort("Can't load", file);

  return(snd);
}


/* --- LOAD A SONG --- */

Mix_Music * load_song(const std::string& file)
{
  if(!audio_device)
    return 0;
  
  Mix_Music * sng;

  sng = Mix_LoadMUS(file.c_str());

  /* printf message and abort if there is an initialized audio device */
  if (sng == NULL)
    st_abort("Can't load", file);
 
  return (sng);
}


/* --- PLAY A SOUND ON LEFT OR RIGHT OR CENTER SPEAKER --- */

void play_sound(Mix_Chunk * snd, enum Sound_Speaker whichSpeaker)
{
  /* this won't call the function if the user has disabled sound
   * either via menu or via command-line option
   */
  if (use_sound && audio_device)
    {
      Mix_PlayChannel( whichSpeaker, snd, 0);

      /* prepare for panning effects for next call */
      /* warning: currently, I do not check for errors here */
      switch (whichSpeaker) {
        case SOUND_LEFT_SPEAKER:
          Mix_SetPanning( SOUND_LEFT_SPEAKER, 230, 24 );
          break;
        case SOUND_RIGHT_SPEAKER:
          Mix_SetPanning( SOUND_RIGHT_SPEAKER, 24, 230 );
          break;
        default:  // keep the compiler happy
          break;
      }
    }
}


void free_chunk(Mix_Chunk *chunk)
{
  if (chunk != NULL)
    {
      DEBUG_MSG( __PRETTY_FUNCTION__ );
      Mix_FreeChunk( chunk );
      chunk = NULL;
    }
}

void halt_music(void)
{
  if (!use_music || !audio_device)
    return;

  Mix_HaltMusic();
  current_song = 0;
}


void play_music(Mix_Music *music)
{
  if (!audio_device)
    return;

  if (use_music && Mix_PlayMusic(music, -1) < 0)
    st_abort("Couldn't play music: ", Mix_GetError());

  current_song = music;
}


void free_music(Mix_Music *music)
{
  Mix_FreeMusic( music );
}

void enable_music(bool enable)
{
  if(!audio_device)
    return;
  
  use_music = enable;
  if(!use_music)
    Mix_HaltMusic();
  else
    Mix_PlayMusic(current_song, -1);
}

