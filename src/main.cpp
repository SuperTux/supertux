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

#include "main.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#ifndef WIN32
#include <libgen.h>
#endif
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

#include "gameconfig.h"
#include "resources.h"
#include "app/globals.h"
#include "app/setup.h"
#include "app/gettext.h"
#include "audio/sound_manager.h"
#include "control/joystickkeyboardcontroller.h"
#include "misc.h"
#include "game_session.h"

SDL_Surface* screen = 0;
JoystickKeyboardController* main_controller = 0;

static void init_config()
{
  config = new Config();
  try {
    config->load();
  } catch(std::exception& e) {
#ifdef DEBUG
    std::cerr << "Couldn't load config file: " << e.what() << "\n";
#endif
  }
}

static void find_directories()
{
  const char* home = getenv("HOME");
  if(home == 0) {
#ifdef DEBUG
    std::cerr << "Couldn't find home directory.\n";
#endif
    home = ".";
  }

  user_dir = home;
  user_dir += "/.supertux";

  // Remove .supertux config file from old versions
  if(FileSystem::faccessible(user_dir)) {
    std::cerr << "Removing old config file " << user_dir << "\n";
    remove(user_dir.c_str());
  }

  // create directories
  std::string savedir = user_dir + "/save";
  mkdir(user_dir.c_str(), 0755);
  mkdir(savedir.c_str(), 0755);

  // try current directory as datadir
  if(datadir.empty()) {
    if(FileSystem::faccessible("./data/credits.txt")) {
      datadir = "./data/";
    }
  }

  // Detect datadir with some linux magic
#ifndef WIN32
  if(datadir.empty()) {
    char exe_file[PATH_MAX];
    if(readlink("/proc/self/exe", exe_file, PATH_MAX) < 0) {
#ifdef DEBUG
      std::cerr << "Couldn't read /proc/self/exe \n";
#endif
    } else {
      std::string exedir = std::string(dirname(exe_file)) + "/";
      std::string testdir = exedir + "./data/";
      if(access(testdir.c_str(), F_OK) == 0) {
        datadir = testdir;
      }
      
      testdir = exedir + "../share/supertux/";
      if(datadir.empty() && access(testdir.c_str(), F_OK) == 0) {
        datadir = testdir;
      }
    }  
  }
#endif
  
#ifdef DATA_PREFIX
  // use default location
  if(datadir.empty()) {
    datadir = DATA_PREFIX;
  }
#endif

  if(datadir.empty())
    throw std::runtime_error("Couldn't find datadir");
}

static void init_tinygettext()
{
  dictionary_manager.add_directory(datadir + "/locale");
  dictionary_manager.set_charset("iso8859-1");
}

static void print_usage(const char* argv0)
{
  fprintf(stderr, _("Usage: %s [OPTIONS] LEVELFILE\n\n"), argv0);
  fprintf(stderr,
          _("Options:\n"
            "  -f, --fullscreen             Run in fullscreen mode.\n"
            "  -w, --window                 Run in window mode.\n"
            "  -g, --geometry WIDTHxHEIGHT  Run SuperTux in give resolution\n"
            "  --help                       Show this help message\n"
            "  --version                    Display SuperTux version and quit\n"
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
      throw std::runtime_error("");
    } else if(arg == "--version") {
      std::cerr << PACKAGE_NAME << " " << PACKAGE_VERSION << "\n";
      throw std::runtime_error("");
    } else if(arg[0] != '-') {
      config->start_level = arg;
    } else {
      std::cerr << "Unknown option '" << arg << "'.\n";
      std::cerr << "Use --help to see a list of options.\n";
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
  SDL_Surface* icon = IMG_Load(
    get_resource_filename("images/supertux.xpm").c_str());
  if(icon != 0) {
    SDL_WM_SetIcon(icon, 0);
    SDL_FreeSurface(icon);
  }
#ifdef DEBUG
  else {
    std::cerr << "Warning: Couldn't find icon 'images/supertux.xpm'.\n";
  }
#endif

  // setup opengl state and transform
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glViewport(0, 0, screen->w, screen->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // logical resolution here not real monitor resolution
  glOrtho(0, 800, 600, 0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);

  check_gl_error();

  Surface::reload_all();
}

static void init_audio()
{
  sound_manager = new SoundManager();
  
  int format = MIX_DEFAULT_FORMAT;
  if(Mix_OpenAudio(config->audio_frequency, format, config->audio_channels,
                   config->audio_chunksize) < 0) {
    std::cerr << "Couldn't initialize audio ("
              << config->audio_frequency << "HZ, " << config->audio_channels
              << " Channels, Format " << format << ", ChunkSize "
              << config->audio_chunksize << "): " << SDL_GetError() << "\n";
    return;
  }
  sound_manager->set_audio_device_available(true);
  sound_manager->enable_sound(config->sound_enabled);
  sound_manager->enable_music(config->music_enabled);
  
  if(Mix_AllocateChannels(config->audio_voices) < 0) {
    std::cerr << "Couldn't allocate '" << config->audio_voices << "' audio voices: "
              << SDL_GetError() << "\n";
    return;
  }
}

static void quit_audio()
{
  if(sound_manager) {
    if(sound_manager->audio_device_available())
      Mix_CloseAudio();

    delete sound_manager;
    sound_manager = 0;
  }
}

int main(int argc, char** argv) 
{
#ifndef DEBUG // we want backtraces in debug mode so don't catch exceptions
  try {
#endif
    srand(time(0));
    init_sdl();
    main_controller = new JoystickKeyboardController();    
    find_directories();
    init_config();
    init_tinygettext();
    parse_commandline(argc, argv);
    init_audio();
    init_video();

    setup_menu();
    load_shared();
    if(config->start_level != "") {
      GameSession session(config->start_level, ST_GL_LOAD_LEVEL_FILE);
      if(config->start_demo != "")
        session.play_demo(config->start_demo);
      if(config->record_demo != "")
        session.record_demo(config->record_demo);
      session.run();
    } else {
      // normal game
      title();
    }    
    
#ifndef DEBUG
  } catch(std::exception& e) {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
    return 1;
  } catch(...) {
    std::cerr << "Unexpected exception." << std::endl;
    return 1;
  }
#endif

  free_menu();
  unload_shared();
#ifdef DEBUG
  Surface::debug_check();
#endif
  quit_audio();

  if(config)
    config->save();
  delete config;
  delete main_controller;
  SDL_Quit();
  
  return 0;
}
