/*
  sound.h
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
 
  April 22, 2000 - March 15, 2004

  Current maintainer:
        Duong-Khang NGUYEN <neoneurone@users.sf.net>
*/

#ifndef SUPERTUX_SOUND_H
#define SUPERTUX_SOUND_H

#include "defines.h"     /* get YES/NO defines */

/* used to reserve some channels for panning effects */
#define SOUND_RESERVED_CHANNELS 2

/*global variable*/
extern int use_sound;           /* handle sound on/off menu and command-line option */
extern int use_music;           /* handle music on/off menu and command-line option */
extern int audio_device;        /* != 0: available and initialized */

/* enum of different internal music types */
enum Music_Type {
  NO_MUSIC,
  LEVEL_MUSIC,
  HURRYUP_MUSIC,
  HERRING_MUSIC
};


/* panning effects: terrible :-) ! */
enum Sound_Speaker {
  SOUND_LEFT_SPEAKER = 0,
  SOUND_RIGHT_SPEAKER = 1,
  SOUND_CENTER_SPEAKER = -1
};

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

extern char* soundfilenames[NUM_SOUNDS];

#ifndef NOSOUND

#include <SDL_mixer.h>

/* variables for stocking the sound and music */
extern Mix_Chunk * sounds[NUM_SOUNDS];
extern Mix_Music * level_song, * level_song_fast, * herring_song;

/* functions handling the sound and music */
int open_audio(int frequency, Uint16 format, int channels, int chunksize);
void close_audio( void );

Mix_Chunk * load_sound(char * file);
void play_sound(Mix_Chunk * snd, enum Sound_Speaker whichSpeaker);
Mix_Music * load_song(char * file);

int playing_music(void);
int halt_music(void);
int play_music(Mix_Music*music, int loops);
void free_music(Mix_Music*music);
void free_chunk(Mix_Chunk*chunk);

int get_current_music();
void set_current_music(int music);
void play_current_music();

#else

/* fake variables */
extern void* sounds[NUM_SOUNDS];
extern void * level_song, * level_song_fast, * herring_song;

/* fake sound handlers */
int open_audio (int frequency, int format, int channels, int chunksize);
void close_audio( void );

void* load_sound(void* file);
void play_sound(void * snd, enum Sound_Speaker whichSpeaker);
void* load_song(void* file);

int playing_music();
void halt_music();
int play_music(void *music, int loops);
void free_music(void *music);
void free_chunk(void *chunk);

int get_current_music();
void set_current_music(int music);
void play_current_music(void);

#endif

#endif /*SUPERTUX_SOUND_H*/
