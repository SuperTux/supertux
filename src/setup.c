/*
  setup.c
  
  Super Tux - Setup
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - February 1st, 2004
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#ifndef NOOPENGL
#include <SDL_opengl.h>
#endif

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "screen.h"
#include "texture.h"

/* Local function prototypes: */

void seticon(void);
void usage(char * prog, int ret);

/* Does the given file exist and is it accessible? */
int faccessible(char *filename)
{
  struct stat filestat;
  if (stat(filename, &filestat) == -1)
    return NO;
  else
    return YES;
}


/* --- SETUP --- */

void st_directory_setup(void)
{

  /* Set SuperTux configuration and save directories */

  /* Get home directory (from $HOME variable)... if we can't determine it,
     use the current directory ("."): */
  char *home;
  if (getenv("HOME") != NULL)
    home = getenv("HOME");
  else
    home = ".";

  st_dir = (char *) malloc(sizeof(char) * (strlen(home) +
                           strlen("/.supertux") + 1));
  strcpy(st_dir, home);
  strcat(st_dir, "/.supertux");

  st_save_dir = (char *) malloc(sizeof(char) * (strlen(st_dir) + strlen("/save") + 1));

  strcpy(st_save_dir,st_dir);
  strcat(st_save_dir,"/save");

  /* Create them. In the case they exist it won't destroy anything. */
#ifdef LINUX

  mkdir(st_dir, 0755);
  mkdir(st_save_dir, 0755);
#else
  #ifdef WIN32

  mkdir(st_dir);
  mkdir(st_save_dir);
#endif
#endif
}

void st_general_setup(void)
{
  /* Seed random number generator: */

  srand(SDL_GetTicks());

  /* Load global images: */
	   
 text_load(&black_text,DATA_PREFIX "/images/status/letters-black.png");
 text_load(&gold_text,DATA_PREFIX "/images/status/letters-gold.png");
 text_load(&blue_text,DATA_PREFIX "/images/status/letters-blue.png");
 text_load(&red_text,DATA_PREFIX "/images/status/letters-red.png");
 
  /* Set icon image: */

  seticon();
  SDL_EnableUNICODE(1);

}

void st_video_setup(void)
{

if(screen != NULL)
   SDL_FreeSurface(screen); 

  /* Init SDL Video: */

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      fprintf(stderr,
              "\nError: I could not initialize video!\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", SDL_GetError());
      exit(1);
    }

  /* Open display: */

  if(use_gl)
  st_video_setup_gl();
  else
  st_video_setup_sdl();
  
  texture_setup();
    
  /* Set window manager stuff: */

  SDL_WM_SetCaption("Super Tux", "Super Tux");

}

void st_video_setup_sdl(void)
{
  if (use_fullscreen == YES)
    {
      screen = SDL_SetVideoMode(640, 480, 16, SDL_FULLSCREEN ) ; /* | SDL_HWSURFACE); */
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
    else
    {
      screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF );

      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nError: I could not set up video for 640x480 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          exit(1);
        }
    }
}

void st_video_setup_gl(void)
{
#ifndef NOOPENGL

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if (use_fullscreen == YES)
    {
      screen = SDL_SetVideoMode(640, 480, 32, SDL_FULLSCREEN | SDL_OPENGL ) ; /* | SDL_HWSURFACE); */
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
    else
    {
      screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_OPENGL | SDL_OPENGLBLIT  );

      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nError: I could not set up video for 640x480 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          exit(1);
        }
    }
    	
	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, screen->w, screen->h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, screen->w, screen->h, 0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);
	
#endif
}

void st_joystick_setup(void)
{

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

}

void st_audio_setup(void)
{

  /* Init SDL Audio silently even if --disable-sound : */

  if (audio_device == YES)
    {
      if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
          /* only print out message if sound or music
             was not disabled at command-line
           */
          if (use_sound == YES || use_music == YES)
            {
              fprintf(stderr,
                      "\nWarning: I could not initialize audio!\n"
                      "The Simple DirectMedia error that occured was:\n"
                      "%s\n\n", SDL_GetError());
            }
          /* keep the programming logic the same :-)
             because in this case, use_sound & use_music' values are ignored
             when there's no available audio device
          */
          use_sound = NO;
          use_music = NO;
          audio_device = NO;
        }
    }


  /* Open sound silently regarless the value of "use_sound": */

  if (audio_device == YES)
    {
      if (open_audio(44100, AUDIO_S16, 2, 2048) < 0)
        {
          /* only print out message if sound or music
             was not disabled at command-line
           */
          if ((use_sound == YES) || (use_music == YES))
            {
              fprintf(stderr,
                      "\nWarning: I could not set up audio for 44100 Hz "
                      "16-bit stereo.\n"
                      "The Simple DirectMedia error that occured was:\n"
                      "%s\n\n", SDL_GetError());
            }
          use_sound = NO;
          use_music = NO;
          audio_device = NO;
        }
    }

}


/* --- SHUTDOWN --- */

void st_shutdown(void)
{
  close_audio();
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


  debug_mode = NO;
  use_fullscreen = NO;
  show_fps = NO;
  use_gl = NO;    

#ifndef NOSOUND

  use_sound = YES;
  use_music = YES;
  audio_device = YES;
#else

  use_sound = NO;
  use_music = NO;
  audio_device = NO;
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
      else if (strcmp(argv[i], "--show-fps") == 0)
        {
          /* Use full screen: */

          show_fps = YES;
        }
      else if (strcmp(argv[i], "--opengl") == 0 ||
          strcmp(argv[i], "-gl") == 0)
        {
	#ifndef NOOPENGL
          /* Use OpengGL: */

          use_gl = YES;
	#endif
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
          /* Disable the compiled in sound feature */
#ifndef NOSOUND
          printf("Sounds disabled \n");
          use_sound = NO;
#else

          printf("Warning: Sounds feature is not compiled in \n");
#endif

        }
      else if (strcmp(argv[i], "--disable-music") == 0)
        {
          /* Disable the compiled in sound feature */
#ifndef NOSOUND
          printf("Music disabled \n");
          use_music = NO;
#else

          printf("Warning: Music feature is not compiled in \n");
#endif

        }
      else if (strcmp(argv[i], "--debug-mode") == 0)
        {
          /* Enable the debug-mode */
          debug_mode = YES;

        }
      else if (strcmp(argv[i], "--help") == 0)
        { 	  /* Show help: */

          printf("Super Tux " VERSION "\n\n");

          printf("----------  Command-line options  ----------\n\n");

          printf("  --opengl            - If opengl support was compiled in, this will enable the EXPERIMENTAL OpenGL mode.\n\n");
	  
          printf("  --disable-sound     - If sound support was compiled in,  this will\n                        disable sound for this session of the game.\n\n");

          printf("  --disable-music     - Like above, but this will disable music.\n\n");

          printf("  --fullscreen        - Run in fullscreen mode.\n\n");

          printf("  --debug-mode        - Enables the debug-mode, which is useful for developers.\n\n");

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

  fprintf(fi, "Usage: %s [--fullscreen] [--opengl] [--disable-sound] [--disable-music] [--debug-mode] | [--usage | --help | --version]\n",
          prog);


  /* Quit! */

  exit(ret);
}

