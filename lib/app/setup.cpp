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

#include <cassert>
#include <cstdio>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>

#include "SDL.h"
#include "SDL_image.h"
#ifndef NOOPENGL
#include "SDL_opengl.h"
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifndef WIN32
#include <libgen.h>
#endif

#include <cctype>

#include "../app/globals.h"
#include "../app/setup.h"
#include "../video/screen.h"
#include "../video/surface.h"
#include "../gui/menu.h"
#include "../utils/configfile.h"
#include "../audio/sound_manager.h"
#include "../app/gettext.h"

using namespace SuperTux;

#ifdef WIN32
#define mkdir(dir, mode)    mkdir(dir)
// on win32 we typically don't want LFS paths
#undef DATA_PREFIX
#define DATA_PREFIX "./data/"
#endif

/* Local function prototypes: */

void seticon(void);
void usage(char * prog, int ret);

/* Does the given file exist and is it accessible? */
int FileSystem::faccessible(const char *filename)
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
int FileSystem::fwriteable(const char *filename)
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
int FileSystem::fcreatedir(const char* relative_dir)
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

/* Get all names of sub-directories in a certain directory. */
/* Returns the number of sub-directories found. */
/* Note: The user has to free the allocated space. */
std::set<std::string> FileSystem::dsubdirs(const char *rel_path,const  char* expected_file)
{
  DIR *dirStructP;
  struct dirent *direntp;
  std::set<std::string> sdirs;
  char filename[1024];
  char path[1024];

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

	      sdirs.insert(direntp->d_name);
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

	      sdirs.insert(direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  return sdirs;
}

std::set<std::string> FileSystem::dfiles(const char *rel_path, const  char* glob, const  char* exception_str)
{
  DIR *dirStructP;
  struct dirent *direntp;
  std::set<std::string> sdirs;
  char path[1024];

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

	      sdirs.insert(direntp->d_name);
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

	      sdirs.insert(direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  return sdirs;
}

void Setup::info(const std::string& _package_name, const std::string& _package_symbol_name, const std::string& _package_version)
{
package_name = _package_name;
package_symbol_name = _package_symbol_name;
package_version = _package_version;
}

/* --- SETUP --- */
/* Set SuperTux configuration and save directories */
void Setup::directories(void)
{
  char *home;
  char str[1024];
  /* Get home directory (from $HOME variable)... if we can't determine it,
     use the current directory ("."): */
  if (getenv("HOME") != NULL)
    home = getenv("HOME");
  else
    home = ".";

  std::string st_dir_tmp = "/." + package_symbol_name;
  st_dir = (char *) malloc(sizeof(char) * (strlen(home) +
                                           strlen(st_dir_tmp.c_str()) + 1));
  strcpy(st_dir, home);
  strcat(st_dir,st_dir_tmp.c_str());

  /* Remove .supertux config-file from old SuperTux versions */
  if(FileSystem::faccessible(st_dir))
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
          
          datadir = exedir + "../../data"; // SuperTux run from source dir
          if (access(datadir.c_str(), F_OK) != 0)
            {
              datadir = exedir + "../share/" + package_symbol_name; // SuperTux run from PATH
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

void Setup::general(void)
{
  /* Seed random number generator: */

  srand(SDL_GetTicks());

  /* Set icon image: */

  seticon();

  /* Unicode needed for input handling: */

  SDL_EnableUNICODE(1);

  /* Load GUI/menu images: */
  checkbox = new Surface(datadir + "/images/status/checkbox.png", true);
  checkbox_checked = new Surface(datadir + "/images/status/checkbox-checked.png", true);
  back = new Surface(datadir + "/images/status/back.png", true);
  arrow_left = new Surface(datadir + "/images/icons/left.png", true);
  arrow_right = new Surface(datadir + "/images/icons/right.png", true);

  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor( datadir + "/images/status/mousecursor.png",1);
  MouseCursor::set_current(mouse_cursor);
  
}

void Setup::general_free(void)
{

  /* Free GUI/menu images: */
  delete checkbox;
  delete checkbox_checked;
  delete back;
  delete arrow_left;
  delete arrow_right;

  /* Free mouse-cursor */
  delete mouse_cursor;
  
}

void Setup::video(unsigned int screen_w, unsigned int screen_h)
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
    video_gl(screen_w, screen_h);
  else
    video_sdl(screen_w, screen_h);

  Surface::reload_all();

  /* Set window manager stuff: */
  SDL_WM_SetCaption((package_name + " " + package_version).c_str(), package_name.c_str());
}

void Setup::video_sdl(unsigned int screen_w, unsigned int screen_h)
{
  if (use_fullscreen)
    {
      screen = SDL_SetVideoMode(screen_w, screen_h, 0, SDL_FULLSCREEN ) ; /* | SDL_HWSURFACE); */
      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nWarning: I could not set up fullscreen video for "
                  "800x600 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          use_fullscreen = false;
        }
    }
  else
    {
      screen = SDL_SetVideoMode(screen_w, screen_h, 0, SDL_HWSURFACE | SDL_DOUBLEBUF );

      if (screen == NULL)
        {
          fprintf(stderr,
                  "\nError: I could not set up video for 800x600 mode.\n"
                  "The Simple DirectMedia error that occured was:\n"
                  "%s\n\n", SDL_GetError());
          exit(1);
        }
    }
}

void Setup::video_gl(unsigned int screen_w, unsigned int screen_h)
{
#ifndef NOOPENGL

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if (use_fullscreen)
    {
      screen = SDL_SetVideoMode(screen_w, screen_h, 0, SDL_FULLSCREEN | SDL_OPENGL) ; /* | SDL_HWSURFACE); */
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
      screen = SDL_SetVideoMode(screen_w, screen_h, 0, SDL_OPENGL);

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

void Setup::joystick(void)
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
          fprintf(stderr, "Info: No joysticks were found.\n");

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

void Setup::audio(void)
{

  /* Init SDL Audio silently even if --disable-sound : */

  if (SoundManager::get()->audio_device_available())
    {
      if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
          /* only print out message if sound or music
             was not disabled at command-line
           */
          if (SoundManager::get()->sound_enabled() || SoundManager::get()->music_enabled())
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
          SoundManager::get()->enable_sound(false);
          SoundManager::get()->enable_music(false);
          SoundManager::get()->set_audio_device_available(false);
        }
    }


  /* Open sound silently regarless the value of "use_sound": */

  if (SoundManager::get()->audio_device_available())
    {
      if (SoundManager::get()->open_audio(44100, AUDIO_S16, 2, 2048) < 0)
        {
          /* only print out message if sound or music
             was not disabled at command-line
           */
          if (SoundManager::get()->sound_enabled() || SoundManager::get()->music_enabled())
            {
              fprintf(stderr,
                      "\nWarning: I could not set up audio for 44100 Hz "
                      "16-bit stereo.\n"
                      "The Simple DirectMedia error that occured was:\n"
                      "%s\n\n", SDL_GetError());
            }
          SoundManager::get()->enable_sound(false);
          SoundManager::get()->enable_music(false);
          SoundManager::get()->set_audio_device_available(false);
        }
    }

}


/* --- SHUTDOWN --- */

void Termination::shutdown(void)
{
  config->save();
  SoundManager::get()->close_audio();
  SDL_Quit();
}

/* --- ABORT! --- */

void Termination::abort(const std::string& reason, const std::string& details)
{
  fprintf(stderr, "\nError: %s\n%s\n\n", reason.c_str(), details.c_str());
  shutdown();
  ::abort();
}

/* Set Icon (private) */

void seticon(void)
{
//  int masklen;
//  Uint8 * mask;
  SDL_Surface * icon;


  /* Load icon into a surface: */

  icon = IMG_Load((datadir + "/images/" + package_symbol_name + ".xpm").c_str());
  if (icon == NULL)
    {
      fprintf(stderr,
              "\nError: I could not load the icon image: %s%s\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", datadir.c_str(), ("/images/" + package_symbol_name + ".xpm").c_str(), SDL_GetError());
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

void Setup::parseargs(int argc, char * argv[])
{
  int i;

  config->load();

  /* Parse arguments: */

  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--fullscreen") == 0 ||
          strcmp(argv[i], "-f") == 0)
        {
          use_fullscreen = true;
        }
      else if (strcmp(argv[i], "--window") == 0 ||
               strcmp(argv[i], "-w") == 0)
        {
          use_fullscreen = false;
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
      else if (strcmp(argv[i], "--worldmap") == 0)
        {
          launch_worldmap_mode = true;
        }
      else if (strcmp(argv[i], "--flip-levels") == 0)
        {
          flip_levels_mode = true;
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
          printf((package_name + package_version + "\n").c_str() );
          exit(0);
        }
      else if (strcmp(argv[i], "--disable-sound") == 0)
        {
          /* Disable the compiled in sound feature */
          printf("Sounds disabled \n");
          SoundManager::get()->enable_sound(false); 
        }
      else if (strcmp(argv[i], "--disable-music") == 0)
        {
          /* Disable the compiled in sound feature */
          printf("Music disabled \n");
          SoundManager::get()->enable_music(false); 
        }
      else if (strcmp(argv[i], "--debug") == 0)
        {
          /* Enable the debug-mode */
          debug_mode = true;

        }
      else if (strcmp(argv[i], "--help") == 0)
        {     /* Show help: */
          puts(_(("  SuperTux  " + package_version + "\n"
               "  Please see the file \"README.txt\" for more details.\n").c_str()));
          printf(_("Usage: %s [OPTIONS] FILENAME\n\n"), argv[0]);
          puts(_("Display Options:\n"
               "  -f, --fullscreen    Run in fullscreen mode.\n"
               "  -w, --window        Run in window mode.\n"
               "  --opengl            If OpenGL support was compiled in, this will tell\n"
               "                      SuperTux to make use of it.\n"
               "  --sdl               Use the SDL software graphical renderer\n"
               "\n"
               "Sound Options:\n"
               "  --disable-sound     If sound support was compiled in,  this will\n"
               "                      disable sound for this session of the game.\n"
               "  --disable-music     Like above, but this will disable music.\n"
               "\n"
               "Misc Options:\n"
               "  -j, --joystick NUM  Use joystick NUM (default: 0)\n" 
               "  --joymap XAXIS:YAXIS:A:B:START\n"
               "                      Define how joystick buttons and axis should be mapped\n"
               "  --leveleditor       Opens the leveleditor in a file.\n"
               "  --worldmap          Opens the specified worldmap file.\n"
               "  --flip-levels       Flip levels upside-down.\n"
               "  -d, --datadir DIR   Load Game data from DIR (default: automatic)\n"
               "  --debug             Enables the debug mode, which is useful for developers.\n"
               "  --help              Display a help message summarizing command-line\n"
               "                      options, license and game controls.\n"
               "  --usage             Display a brief message summarizing command-line options.\n"
               "  --version           Display the version of SuperTux you're running.\n\n"
               ));
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

  fprintf(fi, _("Usage: %s [--fullscreen] [--opengl] [--disable-sound] [--disable-music] [--debug] | [--usage | --help | --version] [--leveleditor] [--worldmap] [--flip-levels] FILENAME\n"),
          prog);


  /* Quit! */

  exit(ret);
}

std::set<std::string> FileSystem::read_directory(const std::string& pathname)
{
  std::set<std::string> dirnames;
  
  DIR* dir = opendir(pathname.c_str());
  if (dir)
    {
      struct dirent *direntp;
      
      while((direntp = readdir(dir)))
        {
          dirnames.insert(direntp->d_name);
        }
      
      closedir(dir);
    }

  return dirnames;
}

/* EOF */
