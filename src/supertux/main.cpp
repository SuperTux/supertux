//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <assert.h>
#include <config.h>

#include "supertux/main.hpp"
#include "util/log.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <cstdlib>
#include <ctime>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef MACOSX
namespace supertux_apple {
#include <CoreFoundation/CoreFoundation.h>
}
#endif

#include "addon/addon_manager.hpp"
#include "audio/sound_manager.hpp"
#include "binreloc/binreloc.h"
#include "control/joystickkeyboardcontroller.hpp"
#include "math/random_generator.hpp"
#include "physfs/physfs_sdl.hpp"
#include "scripting/level.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/mainloop.hpp"
#include "supertux/options_menu.hpp"
#include "supertux/resources.hpp"
#include "supertux/title.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/glutil.hpp"
#include "video/surface.hpp"
#include "video/texture_manager.hpp"
#include "worldmap/worldmap.hpp"

namespace { DrawingContext *context_pointer; }
SDL_Surface *screen;
JoystickKeyboardController* main_controller = 0;
TinyGetText::DictionaryManager dictionary_manager;

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

static void init_config()
{
  config = new Config();
  try {
    config->load();
  } catch(std::exception& e) {
    log_info << "Couldn't load config file: " << e.what() << ", using default settings" << std::endl;
  }
}

static void init_tinygettext()
{
  dictionary_manager.add_directory("locale");
  dictionary_manager.set_charset("UTF-8");

  // Config setting "locale" overrides language detection
  if (config->locale != "") {
    dictionary_manager.set_language( config->locale );
  }
}

static void init_physfs(const char* argv0)
{
  if(!PHYSFS_init(argv0)) {
    std::stringstream msg;
    msg << "Couldn't initialize physfs: " << PHYSFS_getLastError();
    throw std::runtime_error(msg.str());
  }

  // allow symbolic links
  PHYSFS_permitSymbolicLinks(1);

  // Initialize physfs (this is a slightly modified version of
  // PHYSFS_setSaneConfig
  const char* application = "supertux2"; //instead of PACKAGE_NAME so we can coexist with MS1
  const char* userdir = PHYSFS_getUserDir();
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
      log_warning << "Couldn't add '" << dir << "' to physfs searchpath: " << PHYSFS_getLastError() << std::endl;
    } else {
      sourcedir = true;
    }
  }

#ifdef MACOSX
{
  using namespace supertux_apple;

  // when started from Application file on Mac OS X...
  char path[PATH_MAX];
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  assert(mainBundle != 0);
  CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
  assert(mainBundleURL != 0);
  CFStringRef pathStr = CFURLCopyFileSystemPath(mainBundleURL, kCFURLPOSIXPathStyle);
  assert(pathStr != 0);
  CFStringGetCString(pathStr, path, PATH_MAX, kCFStringEncodingUTF8);
  CFRelease(mainBundleURL);
  CFRelease(pathStr);

  dir = std::string(path) + "/Contents/Resources/data";
  testfname = dir + "/credits.txt";
  sourcedir = false;
  f = fopen(testfname.c_str(), "r");
  if(f) {
    fclose(f);
    if(!PHYSFS_addToSearchPath(dir.c_str(), 1)) {
      log_warning << "Couldn't add '" << dir << "' to physfs searchpath: " << PHYSFS_getLastError() << std::endl;
    } else {
      sourcedir = true;
    }
  }
}
#endif

#ifdef _WIN32
  PHYSFS_addToSearchPath(".\\data", 1);
#endif

  if(!sourcedir) {
#if defined(APPDATADIR) || defined(ENABLE_BINRELOC)
    std::string datadir;
#ifdef ENABLE_BINRELOC

    char* dir;
    br_init (NULL);
    dir = br_find_data_dir(APPDATADIR);
    datadir = dir;
    free(dir);

#else
    datadir = APPDATADIR;
#endif
    if(!PHYSFS_addToSearchPath(datadir.c_str(), 1)) {
      log_warning << "Couldn't add '" << datadir << "' to physfs searchpath: " << PHYSFS_getLastError() << std::endl;
    }
#endif
  }

  //show search Path
  char** searchpath = PHYSFS_getSearchPath();
  for(char** i = searchpath; *i != NULL; i++)
    log_info << "[" << *i << "] is in the search path" << std::endl;
  PHYSFS_freeList(searchpath);
}

static void print_usage(const char* argv0)
{
  fprintf(stderr, _("Usage: %s [OPTIONS] [LEVELFILE]\n\n"), argv0);
  fprintf(stderr,
          _("Options:\n"
            "  -f, --fullscreen             Run in fullscreen mode\n"
            "  -w, --window                 Run in window mode\n"
            "  -g, --geometry WIDTHxHEIGHT  Run SuperTux in given resolution\n"
            "  -a, --aspect WIDTH:HEIGHT    Run SuperTux with given aspect ratio\n"
            "  -d, --default                Reset video settings to default values\n"
            "  --renderer RENDERER          Use sdl, opengl, or auto to render\n"
            "  --disable-sfx                Disable sound effects\n"
            "  --disable-music              Disable music\n"
            "  -h, --help                   Show this help message and quit\n"
            "  -v, --version                Show SuperTux version and quit\n"
            "  --console                    Enable ingame scripting console\n"
            "  --noconsole                  Disable ingame scripting console\n"
            "  --show-fps                   Display framerate in levels\n"
            "  --no-show-fps                Do not display framerate in levels\n"
            "  --record-demo FILE LEVEL     Record a demo to FILE\n"
            "  --play-demo FILE LEVEL       Play a recorded demo\n"
            "  -s, --debug-scripts          Enable script debugger.\n"
            "%s\n"), "");
}

/**
 * Options that should be evaluated prior to any initializations at all go here
 */
static bool pre_parse_commandline(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if(arg == "--version" || arg == "-v") {
      std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << std::endl;
      return true;
    }
    if(arg == "--help" || arg == "-h") {
      print_usage(argv[0]);
      return true;
    }
  }

  return false;
}

/**
 * Options that should be evaluated after config is read go here
 */
static bool parse_commandline(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if(arg == "--fullscreen" || arg == "-f") {
      config->use_fullscreen = true;
    } else if(arg == "--default" || arg == "-d") {
      config->use_fullscreen = false;
      
      config->window_width  = 800;
      config->window_height = 600;

      config->fullscreen_width  = 800;
      config->fullscreen_height = 600;

      config->aspect_width  = 0;  // auto detect
      config->aspect_height = 0;
      
    } else if(arg == "--window" || arg == "-w") {
      config->use_fullscreen = false;
    } else if(arg == "--geometry" || arg == "-g") {
      i += 1;
      if(i >= argc) 
        {
          print_usage(argv[0]);
          throw std::runtime_error("Need to specify a size (WIDTHxHEIGHT) for geometry argument");
        } 
      else 
        {
          int width, height;
          if (sscanf(argv[i], "%dx%d", &width, &height) != 2)
            {
              print_usage(argv[0]);
              throw std::runtime_error("Invalid geometry spec, should be WIDTHxHEIGHT");
            }
          else
            {
              config->window_width  = width;
              config->window_height = height;

              config->fullscreen_width  = width;
              config->fullscreen_height = height;
            }
        }
    } else if(arg == "--aspect" || arg == "-a") {
      i += 1;
      if(i >= argc) 
        {
          print_usage(argv[0]);
          throw std::runtime_error("Need to specify a ratio (WIDTH:HEIGHT) for aspect ratio");
        } 
      else 
        {
          int aspect_width  = 0;
          int aspect_height = 0;
          if (strcmp(argv[i], "auto") == 0)
            {
              aspect_width  = 0;
              aspect_height = 0;
            }
          else if (sscanf(argv[i], "%d:%d", &aspect_width, &aspect_height) != 2) 
            {
              print_usage(argv[0]);
              throw std::runtime_error("Invalid aspect spec, should be WIDTH:HEIGHT or auto");
            }
          else 
            {
              float aspect_ratio = static_cast<double>(config->aspect_width) /
                static_cast<double>(config->aspect_height);

              // use aspect ratio to calculate logical resolution
              if (aspect_ratio > 1) {
                config->aspect_width  = static_cast<int> (600 * aspect_ratio + 0.5);
                config->aspect_height = 600;
              } else {
                config->aspect_width  = 600;
                config->aspect_height = static_cast<int> (600 * 1/aspect_ratio + 0.5);
              }
            }
        }
    } else if(arg == "--renderer") {
      i += 1;
      if(i >= argc) 
        {
          print_usage(argv[0]);
          throw std::runtime_error("Need to specify a renderer for renderer argument");
        } 
      else 
        {
          config->video = get_video_system(argv[i]);
        }
    } else if(arg == "--show-fps") {
      config->show_fps = true;
    } else if(arg == "--no-show-fps") {
      config->show_fps = false;
    } else if(arg == "--console") {
      config->console_enabled = true;
    } else if(arg == "--noconsole") {
      config->console_enabled = false;
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
    } else if(arg == "--debug-scripts" || arg == "-s") {
      config->enable_script_debugger = true;
    } else if(arg[0] != '-') {
      config->start_level = arg;
    } else {
      log_warning << "Unknown option '" << arg << "'. Use --help to see a list of options" << std::endl;
      return true;
    }
  }

  return false;
}

static void init_sdl()
{
  if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    std::stringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  // just to be sure
  atexit(SDL_Quit);

  SDL_EnableUNICODE(1);

  // wait 100ms and clear SDL event queue because sometimes we have random
  // joystick events in the queue on startup...
  SDL_Delay(100);
  SDL_Event dummy;
  while(SDL_PollEvent(&dummy))
      ;
}

static void init_rand()
{
  config->random_seed = systemRandom.srand(config->random_seed);

  //const char *how = config->random_seed? ", user fixed.": ", from time().";
  //log_info << "Using random seed " << config->random_seed << how << std::endl;
}

void init_video()
{
  // FIXME: Add something here
  SCREEN_WIDTH  = 800;
  SCREEN_HEIGHT = 600;

  context_pointer->init_renderer();
  screen = SDL_GetVideoSurface();

  SDL_WM_SetCaption(PACKAGE_NAME " " PACKAGE_VERSION, 0);

  // set icon
#ifdef MACOSX
  const char* icon_fname = "images/engine/icons/supertux-256x256.png";
#else
  const char* icon_fname = "images/engine/icons/supertux.xpm";
#endif
  SDL_Surface* icon;
  try {
    icon = IMG_Load_RW(get_physfs_SDLRWops(icon_fname), true);
  } catch (const std::runtime_error& err) {
    icon = 0;
    log_warning << "Couldn't load icon '" << icon_fname << "': " << err.what() << std::endl;
  }
  if(icon != 0) {
    SDL_WM_SetIcon(icon, 0);
    SDL_FreeSurface(icon);
  }
#ifdef DEBUG
  else {
    log_warning << "Couldn't load icon '" << icon_fname << "'" << std::endl;
  }
#endif

  SDL_ShowCursor(0);

  log_info << (config->use_fullscreen?"fullscreen ":"window ")
           << " Window: "     << config->window_width     << "x" << config->window_height
           << " Fullscreen: " << config->fullscreen_width << "x" << config->fullscreen_height
           << " Area: "       << config->aspect_width     << "x" << config->aspect_height << std::endl;
}

static void init_audio()
{
  sound_manager = new SoundManager();

  sound_manager->enable_sound(config->sound_enabled);
  sound_manager->enable_music(config->music_enabled);
}

static void quit_audio()
{
  if(sound_manager != NULL) {
    delete sound_manager;
    sound_manager = NULL;
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

  // clear event queue
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
          main_loop->quit();
          break;
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
    log_info << "Component '" << last_timelog_component <<  "' finished after " << (current_ticks - last_timelog_ticks) / 1000.0 << " seconds" << std::endl;
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
  int result = 0;

  try {

    if(pre_parse_commandline(argc, argv))
      return 0;

    Console::instance = new Console();
    init_physfs(argv[0]);
    init_sdl();

    timelog("controller");
    main_controller = new JoystickKeyboardController();

    timelog("config");
    init_config();

    timelog("addons");
    AddonManager::get_instance().load_addons();

    timelog("tinygettext");
    init_tinygettext();

    timelog("commandline");
    if(parse_commandline(argc, argv))
      return 0;

    timelog("audio");
    init_audio();

    timelog("video");
    DrawingContext context;
    context_pointer = &context;
    init_video();

    Console::instance->init_graphics();

    timelog("scripting");
    Scripting::init_squirrel(config->enable_script_debugger);

    timelog("resources");
    load_shared();

    timelog(0);

    main_loop = new MainLoop();
    if(config->start_level != "") {
      // we have a normal path specified at commandline, not a physfs path.
      // So we simply mount that path here...
      std::string dir = FileSystem::dirname(config->start_level);
      log_debug << "Adding dir: " << dir << std::endl;
      PHYSFS_addToSearchPath(dir.c_str(), true);

      if(config->start_level.size() > 4 &&
              config->start_level.compare(config->start_level.size() - 5, 5, ".stwm") == 0) {
          init_rand();
          main_loop->push_screen(new WorldMapNS::WorldMap(
                      FileSystem::basename(config->start_level)));
      } else {
        init_rand();//If level uses random eg. for
        // rain particles before we do this:
        std::auto_ptr<GameSession> session (
                new GameSession(FileSystem::basename(config->start_level)));

        config->random_seed =session->get_demo_random_seed(config->start_demo);
        init_rand();//initialise generator with seed from session

        if(config->start_demo != "")
          session->play_demo(config->start_demo);

        if(config->record_demo != "")
          session->record_demo(config->record_demo);
        main_loop->push_screen(session.release());
      }
    } else {
      init_rand();
      main_loop->push_screen(new TitleScreen());
    }

    //init_rand(); PAK: this call might subsume the above 3, but I'm chicken!
    main_loop->run(context);
  } catch(std::exception& e) {
    log_fatal << "Unexpected exception: " << e.what() << std::endl;
    result = 1;
  } catch(...) {
    log_fatal << "Unexpected exception" << std::endl;
    result = 1;
  }

  delete main_loop;
  main_loop = NULL;

  unload_shared();
  quit_audio();

  if(config)
    config->save();
  delete config;
  config = NULL;
  delete main_controller;
  main_controller = NULL;
  delete Console::instance;
  Console::instance = NULL;
  Scripting::exit_squirrel();
  delete texture_manager;
  texture_manager = NULL;
  SDL_Quit();
  PHYSFS_deinit();

  return result;
}
