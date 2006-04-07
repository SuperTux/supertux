//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>
#include <assert.h>

#include "msg.hpp"
#include "main.hpp"

#include <stdexcept>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <physfs.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include "gameconfig.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "audio/sound_manager.hpp"
#include "video/surface.hpp"
#include "video/texture_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "misc.hpp"
#include "mainloop.hpp"
#include "title.hpp"
#include "game_session.hpp"
#include "file_system.hpp"
#include "physfs/physfs_sdl.hpp"
#include "exceptions.hpp"

SDL_Surface* screen = 0;
JoystickKeyboardController* main_controller = 0;
TinyGetText::DictionaryManager dictionary_manager;

static void init_config()
{
  config = new Config();
  try {
    config->load();
  } catch(std::exception& e) {
    msg_info("Couldn't load config file: " << e.what() << ", using default settings");
  }
}

static void init_tinygettext()
{
  dictionary_manager.add_directory("locale");
  dictionary_manager.set_charset("UTF-8");
}

static void init_physfs(const char* argv0)
{
  if(!PHYSFS_init(argv0)) {
    std::stringstream msg;
    msg << "Couldn't initialize physfs: " << PHYSFS_getLastError();
    throw std::runtime_error(msg.str());
  }

  // Initialize physfs (this is a slightly modified version of
  // PHYSFS_setSaneConfig
  const char* application = PACKAGE_NAME;
  const char* userdir = PHYSFS_getUserDir();
  const char* dirsep = PHYSFS_getDirSeparator();
  char* writedir = new char[strlen(userdir) + strlen(application) + 2];

  // Set configuration directory
  sprintf(writedir, "%s.%s", userdir, application);
  if(!PHYSFS_setWriteDir(writedir)) {
    // try to create the directory
    char* mkdir = new char[strlen(application) + 2];
    sprintf(mkdir, ".%s", application);
    if(!PHYSFS_setWriteDir(userdir) || !PHYSFS_mkdir(mkdir)) {
      std::ostringstream msg;
      msg << "Failed creating configuration directory '" 
          << writedir << "': " << PHYSFS_getLastError();
      delete[] writedir;
      delete[] mkdir;
      throw std::runtime_error(msg.str());
    }
    delete[] mkdir;
    
    if(!PHYSFS_setWriteDir(writedir)) {
      std::ostringstream msg;
      msg << "Failed to use configuration directory '" 
          <<  writedir << "': " << PHYSFS_getLastError();
      delete[] writedir;
      throw std::runtime_error(msg.str());
    }
  }
  PHYSFS_addToSearchPath(writedir, 0);
  delete[] writedir;

  // Search for archives and add them to the search path
  const char* archiveExt = "zip";
  char** rc = PHYSFS_enumerateFiles("/");
  size_t extlen = strlen(archiveExt);

  for(char** i = rc; *i != 0; ++i) {
    size_t l = strlen(*i);
    if((l > extlen) && ((*i)[l - extlen - 1] == '.')) {
      const char* ext = (*i) + (l - extlen);
      if(strcasecmp(ext, archiveExt) == 0) {
        const char* d = PHYSFS_getRealDir(*i);
        char* str = new char[strlen(d) + strlen(dirsep) + l + 1];
        sprintf(str, "%s%s%s", d, dirsep, *i);
        PHYSFS_addToSearchPath(str, 1);
        delete[] str;
      }
    }
  }
  
  PHYSFS_freeList(rc);

  // when started from source dir...
  std::string dir = PHYSFS_getBaseDir();
  dir += "/data";
  std::string testfname = dir;
  testfname += "/credits.txt";
  bool sourcedir = false;
  FILE* f = fopen(testfname.c_str(), "r");
  if(f) {
    fclose(f);
    if(!PHYSFS_addToSearchPath(dir.c_str(), 1)) {
      msg_warning("Couldn't add '" << dir 
                << "' to physfs searchpath: " << PHYSFS_getLastError());
    } else {
      sourcedir = true;
    }
  }

  if(!sourcedir) {
#if defined(APPDATADIR) || defined(ENABLE_BINRELOC)
    std::string datadir;
#ifdef ENABLE_BINRELOC
    char* brdatadir = br_strcat(DATADIR, "/" PACKAGE_NAME);
    datadir = brdatadir;
    free(brdatadir);
#else
    datadir = APPDATADIR;
#endif
    if(!PHYSFS_addToSearchPath(datadir.c_str(), 1)) {
      msg_warning("Couldn't add '" << datadir
        << "' to physfs searchpath: " << PHYSFS_getLastError());
    }
#endif
  }

  // allow symbolic links
  PHYSFS_permitSymbolicLinks(1);

  //show search Path
  for(char** i = PHYSFS_getSearchPath(); *i != NULL; i++)
    msg_info("[" << *i << "] is in the search path");
}

static void print_usage(const char* argv0)
{
  fprintf(stderr, _("Usage: %s [OPTIONS] [LEVELFILE]\n\n"), argv0);
  fprintf(stderr,
          _("Options:\n"
            "  -f, --fullscreen             Run in fullscreen mode\n"
            "  -w, --window                 Run in window mode\n"
            "  -g, --geometry WIDTHxHEIGHT  Run SuperTux in given resolution\n"
            "  --disable-sfx                Disable sound effects\n"
            "  --disable-music              Disable music\n"
            "  --help                       Show this help message\n"
            "  --version                    Display SuperTux version and quit\n"
            "  --show-fps                   Display framerate in levels\n"
            "  --record-demo FILE LEVEL     Record a demo to FILE\n"
            "  --play-demo FILE LEVEL       Play a recorded demo\n"
            "\n"));
}

static void parse_commandline(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if(arg == "--fullscreen" || arg == "-f") {
      config->use_fullscreen = true;
    } else if(arg == "--window" || arg == "-w") {
      config->use_fullscreen = false;
    } else if(arg == "--geometry" || arg == "-g") {
      if(i+1 >= argc) {
        print_usage(argv[0]);
        throw std::runtime_error("Need to specify a parameter for geometry switch");
      }
      if(sscanf(argv[++i], "%dx%d", &config->screenwidth, &config->screenheight)
         != 2) {
        print_usage(argv[0]);
        throw std::runtime_error("Invalid geometry spec, should be WIDTHxHEIGHT");
      }
    } else if(arg == "--show-fps") {
      config->show_fps = true;
    } else if(arg == "--disable-sfx") {
      config->sound_enabled = false;
    } else if(arg == "--disable-music") {
      config->music_enabled = false;
    } else if(arg == "--play-demo") {
      if(i+1 >= argc) {
        print_usage(argv[0]);
        throw std::runtime_error("Need to specify a demo filename");
      }
      config->start_demo = argv[++i];
    } else if(arg == "--record-demo") {
      if(i+1 >= argc) {
        print_usage(argv[0]);
        throw std::runtime_error("Need to specify a demo filename");
      }
      config->record_demo = argv[++i];
    } else if(arg == "--help") {
      print_usage(argv[0]);
      throw graceful_shutdown();
    } else if(arg == "--version") {
      msg_info(PACKAGE_NAME << " " << PACKAGE_VERSION);
      throw graceful_shutdown();
    } else if(arg[0] != '-') {
      config->start_level = arg;
    } else {
      msg_warning("Unknown option '" << arg << "'. Use --help to see a list of options");
    }
  }

  // TODO joystick switchyes...
}

static void init_sdl()
{
  if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::stringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  SDL_EnableUNICODE(1);

  // wait 100ms and clear SDL event queue because sometimes we have random
  // joystick events in the queue on startup...
  SDL_Delay(100);
  SDL_Event dummy;
  while(SDL_PollEvent(&dummy))
      ;
}

static void check_gl_error()
{
  GLenum glerror = glGetError();
  std::string errormsg;
  
  if(glerror != GL_NO_ERROR) {
    switch(glerror) {
      case GL_INVALID_ENUM:
        errormsg = "Invalid enumeration value";
        break;
      case GL_INVALID_VALUE:
        errormsg = "Numeric argzment out of range";
        break;
      case GL_INVALID_OPERATION:
        errormsg = "Invalid operation";
        break;
      case GL_STACK_OVERFLOW:
        errormsg = "stack overflow";
        break;
      case GL_STACK_UNDERFLOW:
        errormsg = "stack underflow";
        break;
      case GL_OUT_OF_MEMORY:
        errormsg = "out of memory";
        break;
      case GL_TABLE_TOO_LARGE:
        errormsg = "table too large";
        break;
      default:
        errormsg = "unknown error number";
        break;
    }
    std::stringstream msg;
    msg << "OpenGL Error: " << errormsg;
    throw std::runtime_error(msg.str());
  }
}

void init_video()
{
  if(texture_manager != NULL)
    texture_manager->save_textures();
  
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  
  int flags = SDL_OPENGL;
  if(config->use_fullscreen)
    flags |= SDL_FULLSCREEN;
  int width = config->screenwidth;
  int height = config->screenheight;
  int bpp = 0;

  screen = SDL_SetVideoMode(width, height, bpp, flags);
  if(screen == 0) {
    std::stringstream msg;
    msg << "Couldn't set video mode (" << width << "x" << height
        << "-" << bpp << "bpp): " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  SDL_WM_SetCaption(PACKAGE_NAME " " PACKAGE_VERSION, 0);

  // set icon
  SDL_Surface* icon = IMG_Load_RW(
      get_physfs_SDLRWops("images/engine/icons/supertux.xpm"), true);
  if(icon != 0) {
    SDL_WM_SetIcon(icon, 0);
    SDL_FreeSurface(icon);
  }
#ifdef DEBUG
  else {
    msg_warning("Couldn't find icon 'images/engine/icons/supertux.xpm'");
  }
#endif

  // setup opengl state and transform
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, screen->w, screen->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // logical resolution here not real monitor resolution
  glOrtho(0, 800, 600, 0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);

  check_gl_error();

  if(texture_manager != NULL)
    texture_manager->reload_textures();
  else
    texture_manager = new TextureManager();
}

static void init_audio()
{
  sound_manager = new SoundManager();
  
  sound_manager->enable_sound(config->sound_enabled);
  sound_manager->enable_music(config->music_enabled);
}

static void quit_audio()
{
  if(sound_manager) {
    delete sound_manager;
    sound_manager = 0;
  }
}

void wait_for_event(float min_delay, float max_delay)
{
  assert(min_delay <= max_delay);
  
  Uint32 min = (Uint32) (min_delay * 1000);
  Uint32 max = (Uint32) (max_delay * 1000);

  Uint32 ticks = SDL_GetTicks();
  while(SDL_GetTicks() - ticks < min) {
    SDL_Delay(10);
    sound_manager->update();
  }

  // clear even queue
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {}

  /* Handle events: */
  bool running = false;
  ticks = SDL_GetTicks();
  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_QUIT:
          throw graceful_shutdown();
        case SDL_KEYDOWN:
        case SDL_JOYBUTTONDOWN:
        case SDL_MOUSEBUTTONDOWN:
          running = false;
      }
    }
    if(SDL_GetTicks() - ticks >= (max - min))
      running = false;
    sound_manager->update();
    SDL_Delay(10);
  }
}

#ifdef DEBUG
static Uint32 last_timelog_ticks = 0;
static const char* last_timelog_component = 0;

static inline void timelog(const char* component)
{
  Uint32 current_ticks = SDL_GetTicks();
  
  if(last_timelog_component != 0) {
    msg_info("Component '" << last_timelog_component <<  "' finished after " << (current_ticks - last_timelog_ticks) / 1000.0 << " seconds");
  }

  last_timelog_ticks = current_ticks;
  last_timelog_component = component;
}
#else
static inline void timelog(const char* )
{
}
#endif

int main(int argc, char** argv) 
{
  try {
    srand(time(0));
    init_physfs(argv[0]);
    init_sdl();
    timelog("controller");
    main_controller = new JoystickKeyboardController();    
    timelog("config");
    init_config();
    timelog("tinygettext");
    init_tinygettext();
    timelog("commandline");
    parse_commandline(argc, argv);
    timelog("audio");
    init_audio();
    timelog("video");
    init_video();

    timelog("menu");
    setup_menu();
    timelog("resources");
    load_shared();
    timelog(0);

    main_loop = new MainLoop(); 
    if(config->start_level != "") {
      // we have a normal path specified at commandline not physfs paths.
      // So we simply mount that path here...
      std::string dir = FileSystem::dirname(config->start_level);
      PHYSFS_addToSearchPath(dir.c_str(), true);
      GameSession* session
        = new GameSession(
          FileSystem::basename(config->start_level), ST_GL_LOAD_LEVEL_FILE);
      if(config->start_demo != "")
        session->play_demo(config->start_demo);
      if(config->record_demo != "")
        session->record_demo(config->record_demo);
      main_loop->push_screen(session);
    } else {
      main_loop->push_screen(new TitleScreen());
    }

    main_loop->run();

    delete main_loop;
    main_loop = NULL;
  } catch(graceful_shutdown& e) {
  } catch(std::exception& e) {
    msg_fatal("Unexpected exception: " << e.what());
    return 1;
  } catch(...) {
    msg_fatal("Unexpected exception");
    return 1;
  }

  free_menu();
  unload_shared();
  quit_audio();

  if(config)
    config->save();
  delete config;
  delete main_controller;
  delete texture_manager;
  SDL_Quit();
  PHYSFS_deinit();
  
  return 0;
}
