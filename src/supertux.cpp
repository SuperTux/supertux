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

int main(int argc, char * argv[])
{
  int done;
  
  st_directory_setup();
  parseargs(argc, argv);
  
  st_audio_setup();
  st_video_setup();
  st_joystick_setup();
  st_general_setup();
  st_menu();

  if (launch_worldmap_mode)
    {
      worldmap_run();
    }
  else if (level_startup_file)
    {
      gameloop(level_startup_file, 1, ST_GL_LOAD_LEVEL_FILE);
    }
  else
    {  
      done = 0;
  
      while (!done)
        {
          done = title();
        }
    }
  
  clearscreen(0, 0, 0);
  updatescreen();

  st_shutdown();
  
  return(0);
}
