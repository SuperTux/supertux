/*
  sound.c
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
 
  April 22, 2000 - March 15, 2004
*/

#include "defines.h"
#include "globals.h"
#include "sound.h"
#include "setup.h"

/*global variable*/
int use_sound;           /* handle sound on/off menu and command-line option */
int use_music;           /* handle music on/off menu and command-line option */
int audio_device;        /* != 0: available and initialized */
int current_music;

char * soundfilenames[NUM_SOUNDS] = {
                                      DATA_PREFIX "/sounds/jump.wav",
                                      DATA_PREFIX "/sounds/bigjump.wav",
                                      DATA_PREFIX "/sounds/skid.wav",
                                      DATA_PREFIX "/sounds/distro.wav",
                                      DATA_PREFIX "/sounds/herring.wav",
                                      DATA_PREFIX "/sounds/brick.wav",
                                      DATA_PREFIX "/sounds/hurt.wav",
                                      DATA_PREFIX "/sounds/squish.wav",
                                      DATA_PREFIX "/sounds/fall.wav",
                                      DATA_PREFIX "/sounds/ricochet.wav",
                                      DATA_PREFIX "/sounds/bump-upgrade.wav",
                                      DATA_PREFIX "/sounds/upgrade.wav",
                                      DATA_PREFIX "/sounds/excellent.wav",
                                      DATA_PREFIX "/sounds/coffee.wav",
                                      DATA_PREFIX "/sounds/shoot.wav",
                                      DATA_PREFIX "/sounds/lifeup.wav",
                                      DATA_PREFIX "/sounds/stomp.wav",
                                      DATA_PREFIX "/sounds/kick.wav"
                                    };


#ifndef NOSOUND

#include <SDL_mixer.h>

Mix_Chunk * sounds[NUM_SOUNDS];
Mix_Music * level_song, * level_song_fast, * herring_song;

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
  if (audio_device == YES) {
    Mix_UnregisterAllEffects( SOUND_LEFT_SPEAKER );
    Mix_UnregisterAllEffects( SOUND_RIGHT_SPEAKER );
    Mix_CloseAudio();
  }
}


/* --- LOAD A SOUND --- */

Mix_Chunk * load_sound(char * file)
{
  Mix_Chunk * snd;

  snd = Mix_LoadWAV(file);

  /* printf message and abort if there is an initialized audio device */
  if ((snd == NULL) && (audio_device == YES))
    st_abort("Can't load", file);

  return(snd);
}


/* --- LOAD A SONG --- */

Mix_Music * load_song(char * file)
{
  Mix_Music * sng;

  sng = Mix_LoadMUS(file);

  /* printf message and abort if there is an initialized audio device */
  if ((sng == NULL) && (audio_device == YES))
    st_abort("Can't load", file);
  return (sng);
}


/* --- PLAY A SOUND ON LEFT OR RIGHT OR CENTER SPEAKER --- */

void play_sound(Mix_Chunk * snd, enum Sound_Speaker whichSpeaker)
{
  /* this won't call the function if the user has disabled sound
   * either via menu or via command-line option
   */
  if ((use_sound == YES) && (audio_device == YES))
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


int playing_music(void)
{
  if (use_music == YES)
    {
      return Mix_PlayingMusic();
    }
  else
    {
      /* we are in --disable-music we can't be playing music */
      return 0;
    }
}


int halt_music(void)
{
  if ((use_music == YES) && (audio_device == YES))
    {
      return Mix_HaltMusic();
    }
  else
    {
      return 0;
    }
}


int play_music(Mix_Music *music, int loops)
{
  if ((use_music == YES) && (audio_device == YES))
    {
      DEBUG_MSG(__PRETTY_FUNCTION__);
      return Mix_PlayMusic(music, loops);
    }
  else
    {
      /* return error since you're trying to play music in --disable-sound mode */
      return -1;
    }
}


void free_music(Mix_Music *music)
{
  if ( music != NULL )
    {
      DEBUG_MSG(__PRETTY_FUNCTION__);
      Mix_FreeMusic( music );
      music = NULL;
    }
}
 int get_current_music()
  {
 return current_music;
 }
 
 void set_current_music(int music)
 {
 current_music = music;
 }
 
 void play_current_music()
 {
 if(playing_music())
   halt_music();
 
 switch(current_music)
   {
   case LEVEL_MUSIC:
     play_music(level_song, -1);  // -1 to play forever
     break;
   case HERRING_MUSIC:
     play_music(herring_song, -1);
     break;
   case HURRYUP_MUSIC:
     play_music(level_song_fast, -1);
     break;
   case NO_MUSIC:      // keep the compiler happy for the moment :-)
     {}
 /*default:*/
 }
 /* use halt_music whenever you want to stop it */
}

#else

void* sounds[NUM_SOUNDS];
void* level_song, * level_song_fast, * herring_song;

int open_audio (int frequency, int format, int channels, int chunksize)
{
  return -1;
}


void close_audio(void)
{}


void* load_sound(void* file)
{
  return NULL;
}


void play_sound(void * snd, enum Sound_Speaker whichSpeaker)
{}


void* load_song(void* file)
{
  return NULL;
}


int playing_music()
{
  return 0;
}


void halt_music()
{}


int play_music(void *music, int loops)
{
  return 0;
}


void free_music(void *music)
{}


void free_chunk(void *chunk)
{}

int get_current_music()
{
}

void set_current_music(int music)
{
}

void play_current_music(void)
{}

#endif
