/*
  setup.c
  
  Super Tux - Setup
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
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
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "screen.h"
#include "texture.h"
#include "menu.h"
#include "gameloop.h"

/* Local function prototypes: */

void seticon(void);
void usage(char * prog, int ret);

/* Does the given file exist and is it accessible? */
int faccessible(char *filename)
{
  struct stat filestat;
  if (stat(filename, &filestat) == -1)
    {
      return NO;
    }
  else
    {
      if(S_ISREG(filestat.st_mode))
        return YES;
      else
        return NO;
    }
}

/* Can we write to this location? */
int fwriteable(char *filename)
{
  FILE* fi;
  fi = fopen(filename, "wa");
  if (fi == NULL)
    {
      return NO;
    }
  return YES;
}

/* Makes sure a directory is created in either the SuperTux base directory or the SuperTux base directory.*/
int fcreatedir(char* relative_dir)
{
  char path[1024];
  snprintf(path, 1024, "%s/%s/", st_dir, relative_dir);
  if(mkdir(path,0755) != 0)
    {
      snprintf(path, 1024, "%s/%s/", DATA_PREFIX, relative_dir);
      if(mkdir(path,0755) != 0)
        {
          return NO;
        }
      else
        {
          return YES;
        }
    }
  else
    {
      return YES;
    }
}

/* Get all names of sub-directories in a certain directory. */
/* Returns the number of sub-directories found. */
/* Note: The user has to free the allocated space. */
string_list_type dsubdirs(char *rel_path, char* expected_file)
{
  DIR *dirStructP;
  struct dirent *direntp;
  int i = 0;
  string_list_type sdirs;
  char filename[1024];
  char path[1024];

  string_list_init(&sdirs);
  sprintf(path,"%s/%s",st_dir,rel_path);
  if((dirStructP = opendir(path)) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          char absolute_filename[1024];
          struct stat buf;

          sprintf(absolute_filename, "%s/%s", path, direntp->d_name);

          if (stat(absolute_filename, &buf) == 0 && S_ISDIR(buf.st_mode))
            {
              if(expected_file != NULL)
                {
                  sprintf(filename,"%s/%s/%s",path,direntp->d_name,expected_file);
                  if(!faccessible(filename))
                    continue;
                }

              string_list_add_item(&sdirs,direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  sprintf(path,"%s/%s",DATA_PREFIX,rel_path);
  if((dirStructP = opendir(path)) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          char absolute_filename[1024];
          struct stat buf;

          sprintf(absolute_filename, "%s/%s", path, direntp->d_name);

          if (stat(absolute_filename, &buf) == 0 && S_ISDIR(buf.st_mode))
            {
              if(expected_file != NULL)
                {
                  sprintf(filename,"%s/%s/%s",path,direntp->d_name,expected_file);
                  if(!faccessible(filename))
                    {
                      continue;
                    }
                  else
                    {
                      sprintf(filename,"%s/%s/%s/%s",st_dir,rel_path,direntp->d_name,expected_file);
                      if(faccessible(filename))
                        continue;
                    }
                }

              string_list_add_item(&sdirs,direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  return sdirs;
}

string_list_type dfiles(char *rel_path, char* glob, char* exception_str)
{
  DIR *dirStructP;
  struct dirent *direntp;
  int i = 0;
  string_list_type sdirs;
  char filename[1024];
  char path[1024];

  string_list_init(&sdirs);
  sprintf(path,"%s/%s",st_dir,rel_path);
  if((dirStructP = opendir(path)) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          char absolute_filename[1024];
          struct stat buf;

          sprintf(absolute_filename, "%s/%s", path, direntp->d_name);

          if (stat(absolute_filename, &buf) == 0 && S_ISREG(buf.st_mode))
            {
              if(exception_str != NULL)
                {
                  if(strstr(direntp->d_name,exception_str) != NULL)
                    continue;
                }
              if(glob != NULL)
                if(strstr(direntp->d_name,glob) == NULL)
                  continue;

              string_list_add_item(&sdirs,direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  sprintf(path,"%s/%s",DATA_PREFIX,rel_path);
  if((dirStructP = opendir(path)) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          char absolute_filename[1024];
          struct stat buf;

          sprintf(absolute_filename, "%s/%s", path, direntp->d_name);

          if (stat(absolute_filename, &buf) == 0 && S_ISREG(buf.st_mode))
            {
              if(exception_str != NULL)
                {
                  if(strstr(direntp->d_name,exception_str) != NULL)
                    continue;
                }
              if(glob != NULL)
                if(strstr(direntp->d_name,glob) == NULL)
                  continue;

              string_list_add_item(&sdirs,direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  return sdirs;
}

void free_strings(char **strings, int num)
{
  int i;
  for(i=0; i < num; ++i)
    free(strings[i]);
}

/* --- SETUP --- */
/* Set SuperTux configuration and save directories */
void st_directory_setup(void)
{
  char *home;
  char str[1024];
  /* Get home directory (from $HOME variable)... if we can't determine it,
     use the current directory ("."): */
  if (getenv("HOME") != NULL)
    home = getenv("HOME");
  else
    home = ".";

  st_dir = (char *) malloc(sizeof(char) * (strlen(home) +
                           strlen("/.supertux") + 1));
  strcpy(st_dir, home);
  strcat(st_dir, "/.supertux");

  /* Remove .supertux config-file from old SuperTux versions */
  if(faccessible(st_dir))
    {
      remove
        (st_dir);
    }

  st_save_dir = (char *) malloc(sizeof(char) * (strlen(st_dir) + strlen("/save") + 1));

  strcpy(st_save_dir,st_dir);
  strcat(st_save_dir,"/save");

  /* Create them. In the case they exist they won't destroy anything. */
#ifdef LINUX

  mkdir(st_dir, 0755);
  mkdir(st_save_dir, 0755);

  sprintf(str, "%s/levels", st_dir);
  mkdir(str, 0755);
#else
  #ifdef WIN32

  mkdir(st_dir);
  mkdir(st_save_dir);
  sprintf(str, "%s/levels", st_dir);
  mkdir(str);
#endif
#endif

}

/* Create and setup menus. */
void st_menu(void)
{
  menu_init(&main_menu);
  menu_additem(&main_menu,menu_item_create(MN_LABEL,"Main Menu",0,0));
  menu_additem(&main_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&main_menu,menu_item_create(MN_ACTION,"Start Game",0,0));
  menu_additem(&main_menu,menu_item_create(MN_GOTO,"Load Game",0,&load_game_menu));
  menu_additem(&main_menu,menu_item_create(MN_GOTO,"Options",0,&options_menu));
  menu_additem(&main_menu,menu_item_create(MN_ACTION,"Level editor",0,0));
  menu_additem(&main_menu,menu_item_create(MN_ACTION,"Credits",0,0));
  menu_additem(&main_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&main_menu,menu_item_create(MN_ACTION,"Quit",0,0));

  menu_init(&options_menu);
  menu_additem(&options_menu,menu_item_create(MN_LABEL,"Options",0,0));
  menu_additem(&options_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&options_menu,menu_item_create(MN_TOGGLE,"Fullscreen",use_fullscreen,0));
  if(audio_device == YES)
    {
      menu_additem(&options_menu,menu_item_create(MN_TOGGLE,"Sound     ",use_sound,0));
      menu_additem(&options_menu,menu_item_create(MN_TOGGLE,"Music     ",use_music,0));
    }
  else
    {
      menu_additem(&options_menu,menu_item_create(MN_DEACTIVE,"Sound     ",use_sound,0));
      menu_additem(&options_menu,menu_item_create(MN_DEACTIVE,"Music     ",use_music,0));
    }
  menu_additem(&options_menu,menu_item_create(MN_TOGGLE,"Show FPS  ",show_fps,0));
  menu_additem(&options_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&options_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&load_game_menu);
  menu_additem(&load_game_menu,menu_item_create(MN_LABEL,"Load Game",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_DEACTIVE,"Slot 1",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_DEACTIVE,"Slot 2",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_DEACTIVE,"Slot 3",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_DEACTIVE,"Slot 4",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_DEACTIVE,"Slot 5",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&load_game_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&save_game_menu);
  menu_additem(&save_game_menu,menu_item_create(MN_LABEL,"Save Game",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_DEACTIVE,"Slot 1",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_DEACTIVE,"Slot 2",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_DEACTIVE,"Slot 3",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_DEACTIVE,"Slot 4",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_DEACTIVE,"Slot 5",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&save_game_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&game_menu);
  menu_additem(&game_menu,menu_item_create(MN_LABEL,"InGame Menu",0,0));
  menu_additem(&game_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&game_menu,menu_item_create(MN_ACTION,"Return To Game",0,0));
  menu_additem(&game_menu,menu_item_create(MN_GOTO,"Save Game",0,&save_game_menu));
  menu_additem(&game_menu,menu_item_create(MN_GOTO,"Load Game",0,&load_game_menu));
  menu_additem(&game_menu,menu_item_create(MN_GOTO,"Options",0,&options_menu));
  menu_additem(&game_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&game_menu,menu_item_create(MN_ACTION,"Quit Game",0,0));

  menu_init(&highscore_menu);
  menu_additem(&highscore_menu,menu_item_create(MN_TEXTFIELD,"Enter your name:",0,0));

}

void update_load_save_game_menu(menu_type* pmenu, int load)
{
  int i;

  for(i = 2; i < 7; ++i)
    {
      char *tmp;
      slotinfo(&tmp,i-1);
      if(load && strlen(tmp) == strlen("Slot X - Free") )
        pmenu->item[i].kind = MN_DEACTIVE;
      else
        pmenu->item[i].kind = MN_ACTION;
      menu_item_change_text(&pmenu->item[i],tmp);
      free(tmp);
    }
}

void process_save_load_game_menu(int save)
{
  int slot;
  switch (slot = menu_check(save ? &save_game_menu : &load_game_menu))
    {
    default:
      if(slot != -1)
        {
          if(save == YES)
            {
              savegame(slot - 1);
            }
          else
            {
              if(game_started == NO)
                {
                  gameloop("default",slot - 1,ST_GL_LOAD_GAME);
                  show_menu = YES;
                  menu_set_current(&main_menu);
                }
              else
                loadgame(slot - 1);
            }
          st_pause_ticks_stop();
        }
      break;
    }
}

/* Handle changes made to global settings in the options menu. */
void process_options_menu(void)
{
  switch (menu_check(&options_menu))
    {
    case 2:
      if(use_fullscreen != options_menu.item[2].toggled)
        {
          use_fullscreen = !use_fullscreen;
          st_video_setup();
        }
      break;
    case 3:
      if(use_sound != options_menu.item[3].toggled)
        use_sound = !use_sound;
      break;
    case 4:
      if(use_music != options_menu.item[4].toggled)
        {
          if(use_music == YES)
            {
              if(playing_music())
                {
                  halt_music();
                }
              use_music = NO;
            }
          else
            {
              use_music = YES;
              if (!playing_music())
                {
                  play_current_music();
                }
            }
        }
      break;
    case 5:
      if(show_fps != options_menu.item[5].toggled)
        show_fps = !show_fps;
      break;
    }
}

void st_general_setup(void)
{
  /* Seed random number generator: */

  srand(SDL_GetTicks());

  /* Set icon image: */

  seticon();

  /* Unicode needed for input handling: */

  SDL_EnableUNICODE(1);

  /* Load global images: */

  text_load(&black_text,DATA_PREFIX "/images/status/letters-black.png", TEXT_TEXT, 16,18);
  text_load(&gold_text,DATA_PREFIX "/images/status/letters-gold.png", TEXT_TEXT, 16,18);
  text_load(&blue_text,DATA_PREFIX "/images/status/letters-blue.png", TEXT_TEXT, 16,18);
  text_load(&red_text,DATA_PREFIX "/images/status/letters-red.png", TEXT_TEXT, 16,18);
  text_load(&white_text,DATA_PREFIX "/images/status/letters-white.png", TEXT_TEXT, 16,18);
  text_load(&white_small_text,DATA_PREFIX "/images/status/letters-white-small.png", TEXT_TEXT, 8,9);
  text_load(&white_big_text,DATA_PREFIX "/images/status/letters-white-big.png", TEXT_TEXT, 20,23);
  text_load(&yellow_nums,DATA_PREFIX "/images/status/numbers.png", TEXT_NUM, 32,32);

  /* Load GUI/menu images: */
  texture_load(&checkbox, DATA_PREFIX "/images/status/checkbox.png", USE_ALPHA);
  texture_load(&checkbox_checked, DATA_PREFIX "/images/status/checkbox-checked.png", USE_ALPHA);
  texture_load(&back, DATA_PREFIX "/images/status/back.png", USE_ALPHA);
  texture_load(&arrow_left, DATA_PREFIX "/images/icons/left.png", USE_ALPHA);
  texture_load(&arrow_right, DATA_PREFIX "/images/icons/right.png", USE_ALPHA);

}

void st_general_free(void)
{

  /* Free global images: */

  text_free(&black_text);
  text_free(&gold_text);
  text_free(&white_text);
  text_free(&blue_text);
  text_free(&red_text);
  text_free(&white_small_text);
  text_free(&white_big_text);

  /* Free GUI/menu images: */
  texture_free(&checkbox);
  texture_free(&checkbox_checked);
  texture_free(&back);
  texture_free(&arrow_left);
  texture_free(&arrow_right);

  /* Free menus */

  menu_free(&main_menu);
  menu_free(&game_menu);
  menu_free(&options_menu);
  menu_free(&highscore_menu);
  menu_free(&save_game_menu);
  menu_free(&load_game_menu);

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
  SDL_FreeSurface(screen);

  if (use_fullscreen == YES)
    {
      screen = SDL_SetVideoMode(640, 480, 0, SDL_FULLSCREEN ) ; /* | SDL_HWSURFACE); */
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
      screen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE | SDL_DOUBLEBUF );

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
      screen = SDL_SetVideoMode(640, 480, 0, SDL_FULLSCREEN | SDL_OPENGL) ; /* | SDL_HWSURFACE); */
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
      screen = SDL_SetVideoMode(640, 480, 0, SDL_OPENGL);

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
              "%s\n\n", DATA_PREFIX "/images/icon.png", SDL_GetError());
      exit(1);
    }


  /* Create mask: */

  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = (Uint8*) malloc(masklen * sizeof(Uint8));
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

          printf("Warning: Sound capability has not been compiled into this build.\n");
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

