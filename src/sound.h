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

#ifndef NOSOUND

#include <SDL_mixer.h>

// variables for stocking the sound and music
Mix_Chunk* sounds[NUM_SOUNDS];
Mix_Music* song;

// functions handling the sound and music
Mix_Chunk * load_sound(char * file);
void playsound(Mix_Chunk * snd);
Mix_Music * load_song(char * file);

int playing_music(void);
int halt_music(void);
int play_music(Mix_Music*music, int loops);
void free_music(Mix_Music*music);
void free_chunk(Mix_Chunk*chunk);

#else

//fake variables
void* sounds[NUM_SOUNDS];
void* song;

// fake sound handlers
void* load_sound(void* file);
void playsound(void * snd);
void* load_song(void* file);
int Mix_PlayingMusic();
void Mix_HaltMusic();
int Mix_PlayMusic();
void Mix_FreeMusic();
void Mix_FreeChunk();
int Mix_OpenAudio(int a, int b, int c, int d);

int playing_music();
void halt_music();
int play_music(int *music, int loops);
void free_music(int *music);;
void free_chunk(int *chunk);

#endif

#endif /*SUPERTUX_SOUND_H*/
