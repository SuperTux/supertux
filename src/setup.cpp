//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <assert.h>
#include <stdio.h>
#include <iostream>
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

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifndef WIN32
#include <libgen.h>
#endif
#include <ctype.h>

#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "screen.h"
#include "texture.h"
#include "menu.h"
#include "gameloop.h"
#include "configfile.h"
#include "scene.h"
#include "worldmap.h"
#include "resources.h"
#include "intro.h"
#include "music_manager.h"

#include "player.h"

#ifdef WIN32
#define mkdir(dir, mode)    mkdir(dir)
// on win32 we typically don't want LFS paths
#undef DATA_PREFIX
#define DATA_PREFIX "./data/"
#endif

/* Screen proprities: */
/* Don't use this to test for the actual screen sizes. Use screen->w/h instead! */
#define SCREEN_W 640
#define SCREEN_H 480

/* Local function prototypes: */

void seticon(void);
void usage(char * prog, int ret);

/* Does the given file exist and is it accessible? */
int faccessible(const char *filename)
{
  struct stat filestat;
  if (stat(filename, &filestat) == -1)
    {
      return false;
    }
  else
    {
      if(S_ISREG(filestat.st_mode))
        return true;
      else
        return false;
    }
}

/* Can we write to this location? */
int fwriteable(const char *filename)
{
  FILE* fi;
  fi = fopen(filename, "wa");
  if (fi == NULL)
    {
      return false;
    }
  return true;
}

/* Makes sure a directory is created in either the SuperTux home directory or the SuperTux base directory.*/
int fcreatedir(const char* relative_dir)
{
  char path[1024];
  snprintf(path, 1024, "%s/%s/", st_dir, relative_dir);
  if(mkdir(path,0755) != 0)
    {
      snprintf(path, 1024, "%s/%s/", datadir.c_str(), relative_dir);
      if(mkdir(path,0755) != 0)
        {
          return false;
        }
      else
        {
          return true;
        }
    }
  else
    {
      return true;
    }
}

FILE * opendata(const char * rel_filename, const char * mode)
{
  char * filename = NULL;
  FILE * fi;

  filename = (char *) malloc(sizeof(char) * (strlen(st_dir) +
                                             strlen(rel_filename) + 1));

  strcpy(filename, st_dir);
  /* Open the high score file: */

  strcat(filename, rel_filename);

  /* Try opening the file: */
  fi = fopen(filename, mode);

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: Unable to open the file \"%s\" ", filename);

      if (strcmp(mode, "r") == 0)
        fprintf(stderr, "for read!!!\n");
      else if (strcmp(mode, "w") == 0)
        fprintf(stderr, "for write!!!\n");
    }
  free( filename );

  return(fi);
}

/* Get all names of sub-directories in a certain directory. */
/* Returns the number of sub-directories found. */
/* Note: The user has to free the allocated space. */
string_list_type dsubdirs(const char *rel_path,const  char* expected_file)
{
  DIR *dirStructP;
  struct dirent *direntp;
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

  sprintf(path,"%s/%s",datadir.c_str(),rel_path);
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

string_list_type dfiles(const char *rel_path, const  char* glob, const  char* exception_str)
{
  DIR *dirStructP;
  struct dirent *direntp;
  string_list_type sdirs;
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

  sprintf(path,"%s/%s",datadir.c_str(),rel_path);
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
  mkdir(st_dir, 0755);
  mkdir(st_save_dir, 0755);

  sprintf(str, "%s/levels", st_dir);
  mkdir(str, 0755);

  // User has not that a datadir, so we try some magic
  if (datadir.empty())
    {
#ifndef WIN32
      // Detect datadir
      char exe_file[PATH_MAX];
      if (readlink("/proc/self/exe", exe_file, PATH_MAX) < 0)
        {
          puts("Couldn't read /proc/self/exe, using default path: " DATA_PREFIX);
          datadir = DATA_PREFIX;
        }
      else
        {
          std::string exedir = std::string(dirname(exe_file)) + "/";
          
          datadir = exedir + "../data"; // SuperTux run from source dir
          if (access(datadir.c_str(), F_OK) != 0)
            {
              datadir = exedir + "../share/supertux"; // SuperTux run from PATH
              if (access(datadir.c_str(), F_OK) != 0) 
                { // If all fails, fall back to compiled path
                  datadir = DATA_PREFIX; 
                }
            }
        }
#else
  datadir = DATA_PREFIX;
#endif
    }
  printf("Datadir: %s\n", datadir.c_str());
}

/* Create and setup menus. */
void st_menu(void)
{
  main_menu      = new Menu();
  options_menu   = new Menu();
  options_keys_menu     = new Menu();
  options_joystick_menu = new Menu();
  load_game_menu = new Menu();
  save_game_menu = new Menu();
  game_menu      = new Menu();
  highscore_menu = new Menu();
  contrib_menu   = new Menu();
  contrib_subset_menu   = new Menu();
  worldmap_menu  = new Menu();

  main_menu->set_pos(screen->w/2, 335);
  main_menu->additem(MN_GOTO, "Start Game",0,load_game_menu, MNID_STARTGAME);
  main_menu->additem(MN_GOTO, "Contrib Levels",0,contrib_menu, MNID_CONTRIB);
  main_menu->additem(MN_GOTO, "Options",0,options_menu, MNID_OPTIONMENU);
  main_menu->additem(MN_ACTION,"Level Editor",0,0, MNID_LEVELEDITOR);
  main_menu->additem(MN_ACTION,"Credits",0,0, MNID_CREDITS);
  main_menu->additem(MN_ACTION,"Quit",0,0, MNID_QUITMAINMENU);

  options_menu->additem(MN_LABEL,"Options",0,0);
  options_menu->additem(MN_HL,"",0,0);
#ifndef NOOPENGL
  options_menu->additem(MN_TOGGLE,"OpenGL",use_gl,0, MNID_OPENGL);
#else
  options_menu->additem(MN_DEACTIVE,"OpenGL (not supported)",use_gl, 0, MNID_OPENGL);
#endif
  options_menu->additem(MN_TOGGLE,"Fullscreen",use_fullscreen,0, MNID_FULLSCREEN);
  if(audio_device)
    {
      options_menu->additem(MN_TOGGLE,"Sound     ", use_sound,0, MNID_SOUND);
      options_menu->additem(MN_TOGGLE,"Music     ", use_music,0, MNID_MUSIC);
    }
  else
    {
      options_menu->additem(MN_DEACTIVE,"Sound     ", false,0, MNID_SOUND);
      options_menu->additem(MN_DEACTIVE,"Music     ", false,0, MNID_MUSIC);
    }
  options_menu->additem(MN_TOGGLE,"Show FPS  ",show_fps,0, MNID_SHOWFPS);
  options_menu->additem(MN_GOTO,"Keyboard Setup",0,options_keys_menu);

  if(use_joystick)
    options_menu->additem(MN_GOTO,"Joystick Setup",0,options_joystick_menu);

  options_menu->additem(MN_HL,"",0,0);
  options_menu->additem(MN_BACK,"Back",0,0);
  
  options_keys_menu->additem(MN_LABEL,"Key Setup",0,0);
  options_keys_menu->additem(MN_HL,"",0,0);
  options_keys_menu->additem(MN_CONTROLFIELD_KB,"Left move", 0,0, 0,&keymap.left);
  options_keys_menu->additem(MN_CONTROLFIELD_KB,"Right move", 0,0, 0,&keymap.right);
  options_keys_menu->additem(MN_CONTROLFIELD_KB,"Jump", 0,0, 0,&keymap.jump);
  options_keys_menu->additem(MN_CONTROLFIELD_KB,"Duck", 0,0, 0,&keymap.duck);
  options_keys_menu->additem(MN_CONTROLFIELD_KB,"Power/Run", 0,0, 0,&keymap.fire);
  options_keys_menu->additem(MN_HL,"",0,0);
  options_keys_menu->additem(MN_BACK,"Back",0,0);

  if(use_joystick)
    {
    options_joystick_menu->additem(MN_LABEL,"Joystick Setup",0,0);
    options_joystick_menu->additem(MN_HL,"",0,0);
    //options_joystick_menu->additem(MN_CONTROLFIELD_JS,"X axis", 0,0, 0,&joystick_keymap.x_axis);
    //options_joystick_menu->additem(MN_CONTROLFIELD_JS,"Y axis", 0,0, 0,&joystick_keymap.y_axis);
    options_joystick_menu->additem(MN_CONTROLFIELD_JS,"A button", 0,0, 0,&joystick_keymap.a_button);
    options_joystick_menu->additem(MN_CONTROLFIELD_JS,"B button", 0,0, 0,&joystick_keymap.b_button);
    //options_joystick_menu->additem(MN_CONTROLFIELD_JS,"Start", 0,0, 0,&joystick_keymap.start_button);
    //options_joystick_menu->additem(MN_CONTROLFIELD_JS,"DeadZone", 0,0, 0,&joystick_keymap.dead_zone);
    options_joystick_menu->additem(MN_HL,"",0,0);
    options_joystick_menu->additem(MN_BACK,"Back",0,0);
    }
  
  load_game_menu->additem(MN_LABEL,"Start Game",0,0);
  load_game_menu->additem(MN_HL,"",0,0);
  load_game_menu->additem(MN_DEACTIVE,"Slot 1",0,0, 1);
  load_game_menu->additem(MN_DEACTIVE,"Slot 2",0,0, 2);
  load_game_menu->additem(MN_DEACTIVE,"Slot 3",0,0, 3);
  load_game_menu->additem(MN_DEACTIVE,"Slot 4",0,0, 4);
  load_game_menu->additem(MN_DEACTIVE,"Slot 5",0,0, 5);
  load_game_menu->additem(MN_HL,"",0,0);
  load_game_menu->additem(MN_BACK,"Back",0,0);

  save_game_menu->additem(MN_LABEL,"Save Game",0,0);
  save_game_menu->additem(MN_HL,"",0,0);
  save_game_menu->additem(MN_DEACTIVE,"Slot 1",0,0, 1);
  save_game_menu->additem(MN_DEACTIVE,"Slot 2",0,0, 2);
  save_game_menu->additem(MN_DEACTIVE,"Slot 3",0,0, 3);
  save_game_menu->additem(MN_DEACTIVE,"Slot 4",0,0, 4);
  save_game_menu->additem(MN_DEACTIVE,"Slot 5",0,0, 5);
  save_game_menu->additem(MN_HL,"",0,0);
  save_game_menu->additem(MN_BACK,"Back",0,0);

  game_menu->additem(MN_LABEL,"Pause",0,0);
  game_menu->additem(MN_HL,"",0,0);
  game_menu->additem(MN_ACTION,"Continue",0,0,MNID_CONTINUE);
  game_menu->additem(MN_GOTO,"Options",0,options_menu);
  game_menu->additem(MN_HL,"",0,0);
  game_menu->additem(MN_ACTION,"Abort Level",0,0,MNID_ABORTLEVEL);

  worldmap_menu->additem(MN_LABEL,"Pause",0,0);
  worldmap_menu->additem(MN_HL,"",0,0);
  worldmap_menu->additem(MN_ACTION,"Continue",0,0,MNID_RETURNWORLDMAP);
  worldmap_menu->additem(MN_GOTO,"Options",0,options_menu);
  worldmap_menu->additem(MN_HL,"",0,0);
  worldmap_menu->additem(MN_ACTION,"Quit Game",0,0,MNID_QUITWORLDMAP);

  highscore_menu->additem(MN_TEXTFIELD,"Enter your name:",0,0);
}

void update_load_save_game_menu(Menu* pmenu)
{
  for(int i = 2; i < 7; ++i)
    {
      // FIXME: Insert a real savegame struct/class here instead of
      // doing string vodoo
      std::string tmp = slotinfo(i - 1);
      pmenu->item[i].kind = MN_ACTION;
      pmenu->item[i].change_text(tmp.c_str());
    }
}

bool process_load_game_menu()
{
  int slot = load_game_menu->check();

  if(slot != -1 && load_game_menu->get_item_by_id(slot).kind == MN_ACTION)
    {
      char slotfile[1024];
      snprintf(slotfile, 1024, "%s/slot%d.stsg", st_save_dir, slot);

      if (access(slotfile, F_OK) != 0)
        {
          draw_intro();
        }

      fadeout();
      WorldMapNS::WorldMap worldmap;
     
      // Load the game or at least set the savegame_file variable
      worldmap.loadgame(slotfile);

      worldmap.display();
      
      Menu::set_current(main_menu);

      st_pause_ticks_stop();
      return true;
    }
  else
    {
      return false;
    }
}

/* Handle changes made to global settings in the options menu. */
void process_options_menu(void)
{
  switch (options_menu->check())
    {
    case MNID_OPENGL:
#ifndef NOOPENGL
      if(use_gl != options_menu->isToggled(MNID_OPENGL))
        {
          use_gl = !use_gl;
          st_video_setup();
        }
#else
      options_menu->get_item_by_id(MNID_OPENGL).toggled = false;
#endif
      break;
    case MNID_FULLSCREEN:
      if(use_fullscreen != options_menu->isToggled(MNID_FULLSCREEN))
        {
          use_fullscreen = !use_fullscreen;
          st_video_setup();
        }
      break;
    case MNID_SOUND:
      if(use_sound != options_menu->isToggled(MNID_SOUND))
        use_sound = !use_sound;
      break;
    case MNID_MUSIC:
      if(use_music != options_menu->isToggled(MNID_MUSIC))
        {
          use_music = !use_music;
          music_manager->enable_music(use_music);
        }
      break;
    case MNID_SHOWFPS:
      if(show_fps != options_menu->isToggled(MNID_SHOWFPS))
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

  black_text  = new Text(datadir + "/images/status/letters-black.png", TEXT_TEXT, 16,18);
  gold_text   = new Text(datadir + "/images/status/letters-gold.png", TEXT_TEXT, 16,18);
  silver_text = new Text(datadir + "/images/status/letters-silver.png", TEXT_TEXT, 16,18);
  blue_text   = new Text(datadir + "/images/status/letters-blue.png", TEXT_TEXT, 16,18);
  red_text    = new Text(datadir + "/images/status/letters-red.png", TEXT_TEXT, 16,18);
  green_text  = new Text(datadir + "/images/status/letters-green.png", TEXT_TEXT, 16,18);
  white_text  = new Text(datadir + "/images/status/letters-white.png", TEXT_TEXT, 16,18);
  white_small_text = new Text(datadir + "/images/status/letters-white-small.png", TEXT_TEXT, 8,9);
  white_big_text   = new Text(datadir + "/images/status/letters-white-big.png", TEXT_TEXT, 20,22);
  yellow_nums = new Text(datadir + "/images/status/numbers.png", TEXT_NUM, 32,32);

  /* Load GUI/menu images: */
  checkbox = new Surface(datadir + "/images/status/checkbox.png", USE_ALPHA);
  checkbox_checked = new Surface(datadir + "/images/status/checkbox-checked.png", USE_ALPHA);
  back = new Surface(datadir + "/images/status/back.png", USE_ALPHA);
  arrow_left = new Surface(datadir + "/images/icons/left.png", USE_ALPHA);
  arrow_right = new Surface(datadir + "/images/icons/right.png", USE_ALPHA);

  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor( datadir + "/images/status/mousecursor.png",1);
  MouseCursor::set_current(mouse_cursor);
  
}

void st_general_free(void)
{

  /* Free global images: */
  delete black_text;
  delete gold_text;
  delete silver_text;
  delete white_text;
  delete blue_text;
  delete red_text;
  delete green_text;
  delete white_small_text;
  delete white_big_text;
  delete yellow_nums;

  /* Free GUI/menu images: */
  delete checkbox;
  delete checkbox_checked;
  delete back;
  delete arrow_left;
  delete arrow_right;

  /* Free mouse-cursor */
  delete mouse_cursor;
  
  /* Free menus */
  delete main_menu;
  delete game_menu;
  delete options_menu;
  delete highscore_menu;
  delete save_game_menu;
  delete load_game_menu;
}

void st_video_setup(void)
{
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

  Surface::reload_all();

  /* Set window manager stuff: */
  SDL_WM_SetCaption("SuperTux " VERSION, "SuperTux");
}

void st_video_setup_sdl(void)
{
  if (use_fullscreen)
    {
      screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, SDL_FULLSCREEN ) ; /* | SDL_HWSURFACE); */
      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nWarning: I could not set up fullscreen video for "
                  "640x480 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          use_fullscreen = false;
        }
    }
  else
    {
      screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, SDL_HWSURFACE | SDL_DOUBLEBUF );

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

  if (use_fullscreen)
    {
      screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, SDL_FULLSCREEN | SDL_OPENGL) ; /* | SDL_HWSURFACE); */
      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nWarning: I could not set up fullscreen video for "
                  "640x480 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          use_fullscreen = false;
        }
    }
  else
    {
      screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0, SDL_OPENGL);

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

  use_joystick = true;

  if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
      fprintf(stderr, "Warning: I could not initialize joystick!\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", SDL_GetError());

      use_joystick = false;
    }
  else
    {
      /* Open joystick: */
      if (SDL_NumJoysticks() <= 0)
        {
          fprintf(stderr, "Warning: No joysticks are available.\n");

          use_joystick = false;
        }
      else
        {
          js = SDL_JoystickOpen(joystick_num);

          if (js == NULL)
            {
              fprintf(stderr, "Warning: Could not open joystick %d.\n"
                      "The Simple DirectMedia error that occured was:\n"
                      "%s\n\n", joystick_num, SDL_GetError());

              use_joystick = false;
            }
          else
            {
              if (SDL_JoystickNumAxes(js) < 2)
                {
                  fprintf(stderr,
                          "Warning: Joystick does not have enough axes!\n");

                  use_joystick = false;
                }
              else
                {
                  if (SDL_JoystickNumButtons(js) < 2)
                    {
                      fprintf(stderr,
                              "Warning: "
                              "Joystick does not have enough buttons!\n");

                      use_joystick = false;
                    }
                }
            }
        }
    }
}

void st_audio_setup(void)
{

  /* Init SDL Audio silently even if --disable-sound : */

  if (audio_device)
    {
      if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
          /* only print out message if sound or music
             was not disabled at command-line
           */
          if (use_sound || use_music)
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
          use_sound = false;
          use_music = false;
          audio_device = false;
        }
    }


  /* Open sound silently regarless the value of "use_sound": */

  if (audio_device)
    {
      if (open_audio(44100, AUDIO_S16, 2, 2048) < 0)
        {
          /* only print out message if sound or music
             was not disabled at command-line
           */
          if (use_sound || use_music)
            {
              fprintf(stderr,
                      "\nWarning: I could not set up audio for 44100 Hz "
                      "16-bit stereo.\n"
                      "The Simple DirectMedia error that occured was:\n"
                      "%s\n\n", SDL_GetError());
            }
          use_sound = false;
          use_music = false;
          audio_device = false;
        }
    }

}


/* --- SHUTDOWN --- */

void st_shutdown(void)
{
  close_audio();
  SDL_Quit();
  saveconfig();
}

/* --- ABORT! --- */

void st_abort(const std::string& reason, const std::string& details)
{
  fprintf(stderr, "\nError: %s\n%s\n\n", reason.c_str(), details.c_str());
  st_shutdown();
  abort();
}

/* Set Icon (private) */

void seticon(void)
{
//  int masklen;
//  Uint8 * mask;
  SDL_Surface * icon;


  /* Load icon into a surface: */

  icon = IMG_Load((datadir + "/images/supertux.xpm").c_str());
  if (icon == NULL)
    {
      fprintf(stderr,
              "\nError: I could not load the icon image: %s%s\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", datadir.c_str(), "/images/supertux.xpm", SDL_GetError());
      exit(1);
    }


  /* Create mask: */
/*
  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = (Uint8*) malloc(masklen * sizeof(Uint8));
  memset(mask, 0xFF, masklen);
*/

  /* Set icon: */

  SDL_WM_SetIcon(icon, NULL);//mask);


  /* Free icon surface & mask: */

//  free(mask);
  SDL_FreeSurface(icon);
}


/* Parse command-line arguments: */

void parseargs(int argc, char * argv[])
{
  int i;

  loadconfig();

  /* Parse arguments: */

  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--fullscreen") == 0 ||
          strcmp(argv[i], "-f") == 0)
        {
          /* Use full screen: */

          use_fullscreen = true;
        }
      else if (strcmp(argv[i], "--joystick") == 0 || strcmp(argv[i], "-j") == 0)
        {
          assert(i+1 < argc);
          joystick_num = atoi(argv[++i]);
        }
      else if (strcmp(argv[i], "--joymap") == 0)
        {
          assert(i+1 < argc);
          if (sscanf(argv[++i],
                     "%d:%d:%d:%d:%d", 
                     &joystick_keymap.x_axis, 
                     &joystick_keymap.y_axis, 
                     &joystick_keymap.a_button, 
                     &joystick_keymap.b_button, 
                     &joystick_keymap.start_button) != 5)
            {
              puts("Warning: Invalid or incomplete joymap, should be: 'XAXIS:YAXIS:A:B:START'");
            }
          else
            {
              std::cout << "Using new joymap:\n"
                        << "  X-Axis:       " << joystick_keymap.x_axis << "\n"
                        << "  Y-Axis:       " << joystick_keymap.y_axis << "\n"
                        << "  A-Button:     " << joystick_keymap.a_button << "\n"
                        << "  B-Button:     " << joystick_keymap.b_button << "\n"
                        << "  Start-Button: " << joystick_keymap.start_button << std::endl;
            }
        }
      else if (strcmp(argv[i], "--leveleditor") == 0)
        {
          launch_leveleditor_mode = true;
        }
      else if (strcmp(argv[i], "--datadir") == 0 
               || strcmp(argv[i], "-d") == 0 )
        {
          assert(i+1 < argc);
          datadir = argv[++i];
        }
      else if (strcmp(argv[i], "--show-fps") == 0)
        {
          /* Use full screen: */

          show_fps = true;
        }
      else if (strcmp(argv[i], "--opengl") == 0 ||
               strcmp(argv[i], "-gl") == 0)
        {
#ifndef NOOPENGL
          /* Use OpengGL: */

          use_gl = true;
#endif
        }
      else if (strcmp(argv[i], "--sdl") == 0)
          {
            use_gl = false;
          }
      else if (strcmp(argv[i], "--usage") == 0)
        {
          /* Show usage: */

          usage(argv[0], 0);
        }
      else if (strcmp(argv[i], "--version") == 0)
        {
          /* Show version: */
          printf("SuperTux " VERSION "\n");
          exit(0);
        }
      else if (strcmp(argv[i], "--disable-sound") == 0)
        {
          /* Disable the compiled in sound feature */
          printf("Sounds disabled \n");
          use_sound = false;
          audio_device = false;
        }
      else if (strcmp(argv[i], "--disable-music") == 0)
        {
          /* Disable the compiled in sound feature */
          printf("Music disabled \n");
          use_music = false;
        }
      else if (strcmp(argv[i], "--debug-mode") == 0)
        {
          /* Enable the debug-mode */
          debug_mode = true;

        }
      else if (strcmp(argv[i], "--help") == 0)
        {     /* Show help: */
          puts("Super Tux " VERSION "\n"
               "  Please see the file \"README.txt\" for more details.\n");
          printf("Usage: %s [OPTIONS] FILENAME\n\n", argv[0]);
          puts("Display Options:\n"
               "  --fullscreen        Run in fullscreen mode.\n"
               "  --opengl            If opengl support was compiled in, this will enable\n"
               "                      the EXPERIMENTAL OpenGL mode.\n"
               "  --sdl               Use non-opengl renderer\n"
               "\n"
               "Sound Options:\n"
               "  --disable-sound     If sound support was compiled in,  this will\n"
               "                      disable sound for this session of the game.\n"
               "  --disable-music     Like above, but this will disable music.\n"
               "\n"
               "Misc Options:\n"
               "  -j, --joystick NUM  Use joystick NUM (default: 0)\n" 
               "  --joymap XAXIS:YAXIS:A:B:START\n"
               "  --leveleditor       Opens the leveleditor in a file. (Only works when a file is provided.)\n"
               "                      Define how joystick buttons and axis should be mapped\n"
               "  -d, --datadir DIR   Load Game data from DIR (default: automatic)\n"
               "  --debug-mode        Enables the debug-mode, which is useful for developers.\n"
               "  --help              Display a help message summarizing command-line\n"
               "                      options, license and game controls.\n"
               "  --usage             Display a brief message summarizing command-line options.\n"
               "  --version           Display the version of SuperTux you're running.\n\n"
               );
          exit(0);
        }
      else if (argv[i][0] != '-')
        {
          level_startup_file = argv[i];
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

  fprintf(fi, "Usage: %s [--fullscreen] [--opengl] [--disable-sound] [--disable-music] [--debug-mode] | [--usage | --help | --version] [--worldmap] FILENAME\n",
          prog);


  /* Quit! */

  exit(ret);
}

