/*
  setup.c
  
  Super Tux - Setup
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - November 7, 2001
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "screen.h"
#include "sound.h"


/* Local function prototypes: */

void seticon(void);
void usage(char * prog, int ret);


/* --- SETUP --- */

void st_setup(void)
{
  /* Seed random number generator: */
  
  srand(SDL_GetTicks());
  
  
  /* Init SDL Video: */
  
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      fprintf(stderr,
              "\nError: I could not initialize video!\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", SDL_GetError());
      exit(1);
    }


  /* Init Joystick: */
  
#ifdef JOY_YES
  use_joystick = YES;
  
  if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
      fprintf(stderr, "Warning: I could not initialize joystick!\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", SDL_GetError());
      
      use_joystick = NO;
    }
  else
    {
      /* Open joystick: */
      
      if (SDL_NumJoysticks() <= 0)
        {
          fprintf(stderr, "Warning: No joysticks are available.\n");
          
          use_joystick = NO;
        }
      else
        {
          js = SDL_JoystickOpen(0);
          
          if (js == NULL)
            {
              fprintf(stderr, "Warning: Could not open joystick 1.\n"
                      "The Simple DirectMedia error that occured was:\n"
                      "%s\n\n", SDL_GetError());
              
              use_joystick = NO;
            }
          else
            {
              /* Check for proper joystick configuration: */
              
              if (SDL_JoystickNumAxes(js) < 2)
                {
                  fprintf(stderr,
                          "Warning: Joystick does not have enough axes!\n");
                  
                  use_joystick = NO;
                }
              else
                {
                  if (SDL_JoystickNumButtons(js) < 2)
                    {
                      fprintf(stderr,
                              "Warning: "
                              "Joystick does not have enough buttons!\n");
                      
                      use_joystick = NO;
                    }
                }
            }
        }
    }
#endif
  
  
  /* Init SDL Audio: */
  
  if (use_sound == YES)
    {
      if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
          fprintf(stderr,
                  "\nWarning: I could not initialize audio!\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          use_sound = NO;
        }
    }
  
  
  /* Open sound: */
  
  if (use_sound == YES)
    {
      if (open_audio(44100, AUDIO_S16, 2, 512) < 0)
        {
          fprintf(stderr,
                  "\nWarning: I could not set up audio for 44100 Hz "
                  "16-bit stereo.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          use_sound = NO;
        }
    }


  /* Open display: */
  
  if (use_fullscreen == YES)
    {
      screen = SDL_SetVideoMode(640, 480, 16, SDL_FULLSCREEN) ; /* | SDL_HWSURFACE); */
      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nWarning: I could not set up fullscreen video for "
                  "640x480 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          use_fullscreen = NO;
        }
    }
  
  if (use_fullscreen == NO)
    {
      screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
      
      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nError: I could not set up video for 640x480 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          exit(1);
        }
    }
  
  
  /* Load global images: */
  
  letters_black = load_image(DATA_PREFIX "/images/status/letters-black.png",
			     USE_ALPHA);

  letters_gold = load_image(DATA_PREFIX "/images/status/letters-gold.png",
			     USE_ALPHA);

  letters_blue = load_image(DATA_PREFIX "/images/status/letters-blue.png",
			     USE_ALPHA);

  letters_red = load_image(DATA_PREFIX "/images/status/letters-red.png",
			   USE_ALPHA);
  
  
  /* Set icon image: */
  
  seticon();
  
  
  /* Set window manager stuff: */
  
  SDL_WM_SetCaption("Super Tux", "Super Tux");
}


/* --- SHUTDOWN --- */

void st_shutdown(void)
{
  SDL_Quit();
}


/* --- ABORT! --- */

void st_abort(char * reason, char * details)
{
  fprintf(stderr, "\nError: %s\n%s\n\n", reason, details);
  st_shutdown();
  exit(1);
}


/* Set Icon (private) */

void seticon(void)
{
  int masklen;
  Uint8 * mask;
  SDL_Surface * icon;
  
  
  /* Load icon into a surface: */
  
  icon = IMG_Load(DATA_PREFIX "/images/icon.png");
  if (icon == NULL)
    {
      fprintf(stderr,
              "\nError: I could not load the icon image: %s\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", DATA_PREFIX "images/icon.png", SDL_GetError());
      exit(1);
    }
  
  
  /* Create mask: */
  
  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = malloc(masklen * sizeof(Uint8));
  memset(mask, 0xFF, masklen);
  
  
  /* Set icon: */
  
  SDL_WM_SetIcon(icon, mask);
  
  
  /* Free icon surface & mask: */
  
  free(mask);
  SDL_FreeSurface(icon);
}


/* Parse command-line arguments: */

void parseargs(int argc, char * argv[])
{
  int i;
  
  
  /* Set defaults: */
  
  use_fullscreen = NO;
  #ifndef NOSOUND
  use_sound = YES;
  #else
  use_sound = NO;
  #endif
  
  /* Parse arguments: */
  
  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--fullscreen") == 0 ||
	  strcmp(argv[i], "-f") == 0)
	{
	  /* Use full screen: */
	  
	  use_fullscreen = YES;
	}
      else if (strcmp(argv[i], "--usage") == 0)
	{
	  /* Show usage: */
	  
	  usage(argv[0], 0);
	}
      else if (strcmp(argv[i], "--version") == 0)
	{
	  /* Show version: */
	  
	  printf("Super Tux - version " VERSION "\n");
	  exit(0);
	}
       else if (strcmp(argv[i], "--disable-sound") == 0)
  	{
 	  /* Disable the compiled in sound & music feature */
 #ifndef NOSOUND
 	  printf("Sounds and music disabled \n");
 	  use_sound = NO;
 #else
 	  printf("Sounds and music feature is not compiled in \n");
 #endif
  	}
      else if (strcmp(argv[i], "--help") == 0)
	{ 	  /* Show help: */
 
 	  printf("Super Tux " VERSION "\n\n");
 
 	  printf("----------  Command-line options  ----------\n\n");
 
           printf("  --disable-sound     - If sound support was compiled in,  this will\n                        disable it for this session of the game.\n\n");
 
 	  printf("  --fullscreen        - Run in fullscreen mode.\n\n");
 
 	  printf("  --help              - Display a help message summarizing command-line\n                        options, license and game controls.\n\n");
 
 	  printf("  --usage             - Display a brief message summarizing command-line options.\n\n");
 
 	  printf("  --version           - Display the version of SuperTux you're running.\n\n\n");
 
 
 	  printf("----------          License       ----------\n\n");
 	  printf("  This program comes with ABSOLUTELY NO WARRANTY.\n");
 	  printf("  This is free software, and you are welcome to redistribute\n");
 	  printf("  or modify it under certain conditions. See the file \n");
 	  printf("  \"COPYING.txt\" for more details.\n\n\n");
 
 	  printf("----------      Game controls     ----------\n\n");
 	  printf("  Please see the file \"README.txt\"\n\n");
 
 	  exit(0);
      }
      else
	{
	  /* Unknown - complain! */
	  
	  usage(argv[0], 1);
	}
    }
}


/* Display usage: */

void usage(char * prog, int ret)
{
  FILE * fi;
  
  
  /* Determine which stream to write to: */
  
  if (ret == 0)
    fi = stdout;
  else
    fi = stderr;
  
  
  /* Display the usage message: */
  
  fprintf(fi, "Usage: %s [--fullscreen] | [--disable-sound] |  [--usage | --help | --version]\n",
	  prog);
  
  
  /* Quit! */

  exit(ret);
}
