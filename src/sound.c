/*
  sound.c
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
 
  April 22, 2000 - December 27, 2003
*/

/*
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
*/

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "sound.h"
#include "setup.h"

#ifndef NOSOUND

#include <SDL_mixer.h>

/* --- OPEN THE AUDIO DEVICE --- */

int open_audio (int frequency, Uint16 format, int channels, int chunksize)
{
  return Mix_OpenAudio( frequency, format, channels, chunksize );
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


/* --- PLAY A SOUND --- */

void play_sound(Mix_Chunk * snd)
{
  /* this won't call the function if the user has disabled sound
   * either via menu or via command-line option
   */
  if ((use_sound == YES) && (audio_device == YES))
    {
      Mix_PlayChannel(-1, snd, 0);
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


void set_music_position(int pos)
{
  if ((use_music == YES) && (audio_device == YES))
  {
    if (Mix_PlayingMusic())
    {
      Mix_SetMusicPosition(pos);
    }
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

#else

int open_audio (int frequency, int format, int channels, int chunksize)
{
  return -1;
}

void* load_sound(void* file)
{
  return NULL;
}
void play_sound(void * snd)
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

#endif
