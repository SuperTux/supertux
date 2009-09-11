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
#ifdef GP2X
#include <string.h>
#endif

/*global variable*/
bool use_sound = true;    /* handle sound on/off menu and command-line option */
bool use_music = true;    /* handle music on/off menu and command-line option */
bool audio_device = true; /* != 0: available and initialized */

const char * soundfilenames[NUM_SOUNDS] = {
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
                                       "/sounds/kick.wav",
                                       "/sounds/explode.wav",
                                       "/sounds/warp.wav"
                                    };


#include <SDL_mixer.h>

#ifndef GP2X
Mix_Chunk * sounds[NUM_SOUNDS];
#else

#include <mikmod.h>
static MODULE *music=NULL;
static SAMPLE *chunk[NUM_SOUNDS];
static int chunkFlag[NUM_SOUNDS];
#endif

/* --- OPEN THE AUDIO DEVICE --- */

int open_audio (int frequency, Uint16 format, int channels, int chunksize)
{
//  close_audio();
#ifndef GP2X
  if (Mix_OpenAudio( frequency, format, channels, chunksize ) < 0)
    return -1;

  // allocate 16 channels for mixing
  if (Mix_AllocateChannels(8)  != 8)
    return -2;
  
  /* reserve some channels and register panning effects */
  if (Mix_ReserveChannels(SOUND_RESERVED_CHANNELS) != SOUND_RESERVED_CHANNELS)
    return -3;

  /* prepare the spanning effects */
  Mix_SetPanning( SOUND_LEFT_SPEAKER, 230, 24 );
  Mix_SetPanning( SOUND_RIGHT_SPEAKER, 24, 230 );
#else
  if (drv_oss.Name)  // Valid OSS driver
  {
    if (drv_oss.CommandLine)  // Valid Commandline
    {
      drv_oss.CommandLine("buffer=14,count=2");
    }
    MikMod_RegisterDriver(&drv_oss);
  }
  if (drv_alsa.Name)  // Valid ALSA driver
  {
    if (drv_alsa.CommandLine)  // Valid Commandline
    {
      drv_alsa.CommandLine("buffer=14");
    }
    MikMod_RegisterDriver(&drv_alsa);
  }
  MikMod_RegisterDriver(&drv_nos);

  // register standard tracker
  MikMod_RegisterAllLoaders();

  // Note, the md_mode flags are already set by default
  md_mode |= DMODE_SOFT_SNDFX | DMODE_SOFT_MUSIC;

   if (MikMod_Init(""))  // Command paramenters are ignored as all drivers are registered
  {
    printf("mikmod init war fürn arsch\n");
    return 1;
  }

  load_sounds();

  // get ready to play
  MikMod_EnableOutput();
  md_volume =64;
  md_sndfxvolume = 64;
  //md_musicvolume = md_sndfxvolume = 64;

  use_sound = true;
  audio_device=true;

#endif
  return 0;

}

/* --- CLOSE THE AUDIO DEVICE --- */

void close_audio( void )
{
#ifndef GP2X
  if (audio_device) {
    Mix_UnregisterAllEffects( SOUND_LEFT_SPEAKER );
    Mix_UnregisterAllEffects( SOUND_RIGHT_SPEAKER );
    Mix_CloseAudio();
  }
#else
  int i;
  if (! audio_device) return;

  MikMod_DisableOutput();
  Player_Stop();
  MikMod_Update();

  if (music) {
	  Player_Free(music);
	  music = NULL;
  }

  for ( i=0 ; i<NUM_SOUNDS ; i++ ) {
    if ( chunk[i] ) {
      Sample_Free(chunk[i]);
	}
  }

  MikMod_Update();
  MikMod_Exit();
#endif
}
#ifdef GP2X
void updateSound ( void ) {
  if (! audio_device) return;
  MikMod_Update();
}
#endif


/* --- LOAD A SOUND --- */

Mix_Chunk* load_sound(const std::string& file)
{
  if(!audio_device)
    return 0;
  
  Mix_Chunk* snd = Mix_LoadWAV(file.c_str());

  if (snd == 0)
    st_abort("Can't load", file);

  return(snd);
}
#ifdef GP2X
static void load_sounds() {
  int i;
  std::string name;
  char file[100];

  for ( i=0 ; i<NUM_SOUNDS ; i++ ) {
    name = datadir + soundfilenames[i];
    printf("loading: %s\n",name.c_str());
    snprintf(file,sizeof(file),"%s",name.c_str());
    if ( NULL == (chunk[i] = Sample_Load(file)) ) {
      fprintf(stderr, "Couldn't load: %s\n", file);
      use_sound = 0;
      return;
    }
    chunkFlag[i] = 0;
  }

  // reserve voices for sound effects
  MikMod_SetNumVoices(-1, 4);
}
#endif


/* --- PLAY A SOUND ON LEFT OR RIGHT OR CENTER SPEAKER --- */

void play_sound(Mix_Chunk * snd, enum Sound_Speaker whichSpeaker)
{
  /* this won't call the function if the user has disabled sound
   * either via menu or via command-line option
   */
  if(!audio_device || !use_sound)
    return;

  Mix_PlayChannel( whichSpeaker, snd, 0);

  /* prepare for panning effects for next call */
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

#ifdef GP2X
void play_chunk(int idx) 
{
    int cid;
//  if (use_sound) return;
     cid = Sample_Play (chunk[idx], 0, 0);
  Voice_SetPanning(cid, PAN_CENTER);
  Voice_SetVolume(cid, 1000);
}
#endif


void free_chunk(Mix_Chunk *chunk)
{
  Mix_FreeChunk( chunk );
}

void sound_volume ( int vol )
{
#ifndef GP2X
    static int volume = 10;
    
    if ( vol == 1 ) volume-=5;
    else if ( vol == 2 ) volume+=5;
    Mix_Volume(-1,volume);
#endif
}
#ifdef GP2X
void increaseSoundVolume(void)
{
	if (md_volume > (256 - 13)) md_volume =  256;
	else md_volume += 13;
}

void decreaseSoundVolume(void)
{
	if (md_volume < 13) md_volume = 0;
	else md_volume -= 13;
}
#endif
