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
  Mix_Music * sng;

  sng = Mix_LoadMUS(file.c_str());

  /* printf message and abort if there is an initialized audio device */
  if ((sng == NULL) && audio_device)
    st_abort("Can't load", file);
  return (sng);
}


/* --- PLAY A SOUND ON LEFT OR RIGHT OR CENTER SPEAKER --- */

void play_sound(Mix_Chunk * snd, enum Sound_Speaker whichSpeaker)
{
  /* this won't call the function if the user has disabled sound
   * either via menu or via command-line option
   */
  if ((use_sound == true) && (audio_device == true))
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
  if (use_music == true)
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
  if ((use_music == true) && (audio_device == true))
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
  if ((use_music == true) && (audio_device == true))
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

