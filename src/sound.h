/*
  sound.h
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/

  April 22, 2000 - July 15, 2002
*/

#ifndef SUPERTUX_SOUND_H
#define SUPERTUX_SOUND_H

 /*all the sounds we have*/
#define NUM_SOUNDS 16

/*global variable*/
int use_sound;

/* enum of different internal music types */
enum Music_Type {
        NO_MUSIC,
        LEVEL_MUSIC,
        HURRYUP_MUSIC,
        HERRING_MUSIC
} current_music;


#ifndef NOSOUND

#include <SDL_mixer.h>

/* variables for stocking the sound and music */
Mix_Chunk* sounds[NUM_SOUNDS];
Mix_Music* level_song, *herring_song;

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
void free_music(void *music);;
void free_chunk(void *chunk);

#endif

#endif /*SUPERTUX_SOUND_H*/
