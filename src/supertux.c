/*
  supertux.c
  
  Super Tux
  
  by Bill Kendrick & Tobias Glaesser <tobi.web@gmx.de>
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/


#include "supertux.h"


/* --- MAIN --- */

#ifdef WIN32
main(int argc, char * argv[])
#else
int main(int argc, char * argv[])
#endif
{
  int done;
  
  parseargs(argc, argv);
  
  st_directory_setup();
  st_audio_setup();
  st_video_setup();
  st_joystick_setup();
  st_general_setup();
  st_menu();
  
  done = intro();
  
  done = 0;
  
  while (!done)
    {
      done = title();
    }
  
  clearscreen(0, 0, 0);
  updatescreen();
  
  st_shutdown();
  
  return(0);
}
