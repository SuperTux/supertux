/*
  sound.h
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
 
  April 22, 2000 - December 28, 2003

  Current maintainer:
        Duong-Khang NGUYEN <neoneurone@users.sf.net>
*/

#ifndef SUPERTUX_SOUND_H
#define SUPERTUX_SOUND_H

#include "defines.h"     /* get YES/NO defines */

/*global variable*/
int use_sound;
int use_music;
int audio_device;        /* != 0: available and initialized */

/* enum of different internal music types */
enum Music_Type {
  NO_MUSIC,
  LEVEL_MUSIC,
  HURRYUP_MUSIC,
  HERRING_MUSIC
} current_music;

/* Sound files: */

enum {
  SND_JUMP,
  SND_BIGJUMP,
  SND_SKID,
  SND_DISTRO,
  SND_HERRING,
  SND_BRICK,
  SND_HURT,
  SND_SQUISH,
  SND_FALL,
  SND_RICOCHET,
  SND_BUMP_UPGRADE,
  SND_UPGRADE,
  SND_EXCELLENT,
  SND_COFFEE,
  SND_SHOOT,
  SND_LIFEUP,
  SND_STOMP,
  SND_KICK,
  NUM_SOUNDS
};


static char * soundfilenames[NUM_SOUNDS] = {
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

/* variables for stocking the sound and music */
Mix_Chunk * sounds[NUM_SOUNDS];
Mix_Music * level_song, * level_song_fast, * herring_song;

/* functions handling the sound and music */
int open_audio(int frequency, Uint16 format, int channels, int chunksize);

Mix_Chunk * load_sound(char * file);
void play_sound(Mix_Chunk * snd);
Mix_Music * load_song(char * file);

int playing_music(void);
int halt_music(void);
int play_music(Mix_Music*music, int loops);
void free_music(Mix_Music*music);
void free_chunk(Mix_Chunk*chunk);

#else

//fake variables
void* sounds[NUM_SOUNDS];
void* level_song, *herring_song;

// fake sound handlers
int open_audio (int frequency, int format, int channels, int chunksize);

void* load_sound(void* file);
void play_sound(void * snd);
void* load_song(void* file);

int playing_music();
void halt_music();
int play_music(void *music, int loops);
void free_music(void *music);
;
void free_chunk(void *chunk);

#endif

#endif /*SUPERTUX_SOUND_H*/
