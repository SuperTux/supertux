/*
  supertux.c
  
  Super Tux
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - December 29, 2003
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
  
  done = intro();
  
  done = 0;
  
  while (!done)
    {
      done = title();
      if (!done)
      {
      if(game_started)
	done = gameloop();
      else if(level_editor_started)
        done = leveleditor(1);
      }
    }
  
  clearscreen(0, 0, 0);
  updatescreen();
  
  st_shutdown();
  
  return(0);
}
