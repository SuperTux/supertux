/*
  sound.c
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
 
  April 22, 2000 - July 15, 2002
*/

#ifndef NOSOUND

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#include <SDL_mixer.h>

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "sound.h"
#include "setup.h"


/* --- LOAD A SOUND --- */

Mix_Chunk * load_sound(char * file)
{
  Mix_Chunk * snd;
  
  if (use_sound)
  {
    snd = Mix_LoadWAV(file);
  
    if (snd == NULL)
      st_abort("Can't load", file);
  }
  else
    snd = NULL;

  return(snd);
}


/* --- PLAY A SOUND --- */

void playsound(Mix_Chunk * snd)
{
  Mix_PlayChannel(-1, snd, 0);
}


/* --- LOAD A SONG --- */

Mix_Music * load_song(char * file)
{
  Mix_Music * sng;

  if (use_sound)
  {
    sng = Mix_LoadMUS(file);

    if (sng == NULL)
      st_abort("Can't load", file);
  }
  else
    sng = NULL;

  return (sng);
}

#endif
