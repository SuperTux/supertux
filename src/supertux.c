/*
  supertux.c
  
  Super Tux
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 12, 2000
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "intro.h"
#include "title.h"
#include "gameloop.h"
#include "screen.h"


/* --- MAIN --- */

#ifdef WIN32
main(int argc, char * argv[])
#else
int main(int argc, char * argv[])
#endif
{
  int done;
  
  parseargs(argc, argv);
  
  st_setup();
  
  done = intro();
  
  while (!done)
    {
      done = title();
      if (!done)
	done = gameloop();
    }
  
  clearscreen(0, 0, 0);
  updatescreen();
  
  st_shutdown();
  
  return(0);
}
