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

#include <config.h>
#include <version.h>

#include <SDL_image.h>
#include <physfs.h>
#include <iostream>
#include <binreloc.h>
#include <tinygettext/log.hpp>
#include <boost/format.hpp>

#include "supertux/main.hpp"

#ifdef MACOSX
namespace supertux_apple {
#  include <CoreFoundation/CoreFoundation.h>
} // namespace supertux_apple
#endif

#include "addon/addon_manager.hpp"
#include "audio/sound_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "math/random_generator.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/physfs_sdl.hpp"
#include "physfs/physfs_file_system.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/player_status.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/resources.hpp"
#include "supertux/title_screen.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "worldmap/worldmap.hpp"

namespace { DrawingContext *context_pointer; }

#ifdef _WIN32
# define WRITEDIR_NAME PACKAGE_NAME
#else
# define WRITEDIR_NAME "." PACKAGE_NAME
#endif

void 
Main::init_config()
{
  g_config = new Config();
  try {
    g_config->load();
  } catch(std::exception& e) {
    log_info << "Couldn't load config file: " << e.what() << ", using default settings" << std::endl;
  }
}

void
Main::init_tinygettext()
{
  dictionary_manager = new tinygettext::DictionaryManager();
  tinygettext::Log::set_log_info_callback(0);
  dictionary_manager->set_filesystem(std::auto_ptr<tinygettext::FileSystem>(new PhysFSFileSystem));

  dictionary_manager->add_directory("locale");
  dictionary_manager->set_charset("UTF-8");

  // Config setting "locale" overrides language detection
  if (g_config->locale != "") 
  {
    dictionary_manager->set_language(tinygettext::Language::from_name(g_config->locale));
  }
}

void
Main::init_physfs(const char* argv0)
{
  if(!PHYSFS_init(argv0)) {
    std::stringstream msg;
    msg << "Couldn't initialize physfs: " << PHYSFS_getLastError();
    throw std::runtime_error(msg.str());
  }

  // allow symbolic links
  PHYSFS_permitSymbolicLinks(1);

  // Initialize physfs (this is a slightly modified version of
  // PHYSFS_setSaneConfig)
  const char *env_writedir;
  std::string writedir;

  if ((env_writedir = getenv("SUPERTUX2_USER_DIR")) != NULL) {
    writedir = env_writedir;
    if(!PHYSFS_setWriteDir(writedir.c_str())) {
      std::ostringstream msg;
      msg << "Failed to use configuration directory '"
          <<  writedir << "': " << PHYSFS_getLastError();
      throw std::runtime_error(msg.str());
    }

  } else {
    std::string userdir = PHYSFS_getUserDir();

    // Set configuration directory
    writedir = userdir + WRITEDIR_NAME;
    if(!PHYSFS_setWriteDir(writedir.c_str())) {
      // try to create the directory
      if(!PHYSFS_setWriteDir(userdir.c_str()) || !PHYSFS_mkdir(WRITEDIR_NAME)) {
        std::ostringstream msg;
        msg << "Failed creating configuration directory '"
            << writedir << "': " << PHYSFS_getLastError();
        throw std::runtime_error(msg.str());
      }

      if(!PHYSFS_setWriteDir(writedir.c_str())) {
        std::ostringstream msg;
        msg << "Failed to use configuration directory '"
            <<  writedir << "': " << PHYSFS_getLastError();
        throw std::runtime_error(msg.str());
      }
    }
  }
  PHYSFS_addToSearchPath(writedir.c_str(), 0);

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
    std::string datadir = PHYSFS_getBaseDir();
    datadir = datadir.substr(0, datadir.rfind(INSTALL_SUBDIR_BIN));
    datadir += "/" INSTALL_SUBDIR_SHARE;
#ifdef ENABLE_BINRELOC

    char* dir;
    br_init (NULL);
    dir = br_find_data_dir(datadir.c_str());
    datadir = dir;
    free(dir);

#endif
    if(!PHYSFS_addToSearchPath(datadir.c_str(), 1)) {
      log_warning << "Couldn't add '" << datadir << "' to physfs searchpath: " << PHYSFS_getLastError() << std::endl;
    }
  }

  //show search Path
  char** searchpath = PHYSFS_getSearchPath();
  for(char** i = searchpath; *i != NULL; i++)
    log_info << "[" << *i << "] is in the search path" << std::endl;
  PHYSFS_freeList(searchpath);
}

void
Main::print_usage(const char* argv0)
{
  std::string default_user_data_dir =
      std::string(PHYSFS_getUserDir()) + WRITEDIR_NAME;

  std::cerr << boost::format(_(
                 "\n"
                 "Usage: %s [OPTIONS] [LEVELFILE]\n\n"
                 "Options:\n"
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
                 "\n"
                 "Environment variables:\n"
                 "  SUPERTUX2_USER_DIR           Directory for user data (savegames, etc.);\n"
                 "                               default %s\n"
                 "\n"
                 ))
            % argv0 % default_user_data_dir
            << std::flush;
}

/**
 * Options that should be evaluated prior to any initializations at all go here
 */
bool
Main::pre_parse_commandline(int argc, char** argv)
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
bool
Main::parse_commandline(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if(arg == "--fullscreen" || arg == "-f") {
      g_config->use_fullscreen = true;
    } else if(arg == "--default" || arg == "-d") {
      g_config->use_fullscreen = false;
      
      g_config->window_size     = Size(800, 600);
      g_config->fullscreen_size = Size(800, 600);
      g_config->aspect_size     = Size(0, 0);  // auto detect
      
    } else if(arg == "--window" || arg == "-w") {
      g_config->use_fullscreen = false;
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
          g_config->window_size     = Size(width, height);
          g_config->fullscreen_size = Size(width, height);
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
          float aspect_ratio = static_cast<float>(aspect_width) / static_cast<float>(aspect_height);

          // use aspect ratio to calculate logical resolution
          if (aspect_ratio > 1) {
            g_config->aspect_size = Size(static_cast<int>(600 * aspect_ratio + 0.5),
                                         600);
          } else {
            g_config->aspect_size = Size(600, 
                                         static_cast<int>(600 * 1/aspect_ratio + 0.5));
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
        g_config->video = VideoSystem::get_video_system(argv[i]);
      }
    } else if(arg == "--show-fps") {
      g_config->show_fps = true;
    } else if(arg == "--no-show-fps") {
      g_config->show_fps = false;
    } else if(arg == "--console") {
      g_config->console_enabled = true;
    } else if(arg == "--noconsole") {
      g_config->console_enabled = false;
    } else if(arg == "--disable-sfx") {
      g_config->sound_enabled = false;
    } else if(arg == "--disable-music") {
      g_config->music_enabled = false;
    } else if(arg == "--play-demo") {
      if(i+1 >= argc) {
        print_usage(argv[0]);
        throw std::runtime_error("Need to specify a demo filename");
      }
      g_config->start_demo = argv[++i];
    } else if(arg == "--record-demo") {
      if(i+1 >= argc) {
        print_usage(argv[0]);
        throw std::runtime_error("Need to specify a demo filename");
      }
      g_config->record_demo = argv[++i];
    } else if(arg == "--debug-scripts" || arg == "-s") {
      g_config->enable_script_debugger = true;
    } else if(arg[0] != '-') {
      g_config->start_level = arg;
    } else {
      log_warning << "Unknown option '" << arg << "'. Use --help to see a list of options" << std::endl;
      return true;
    }
  }

  return false;
}

void
Main::init_sdl()
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

void
Main::init_rand()
{
  g_config->random_seed = gameRandom.srand(g_config->random_seed);

  graphicsRandom.srand(0);

  //const char *how = config->random_seed? ", user fixed.": ", from time().";
  //log_info << "Using random seed " << config->random_seed << how << std::endl;
}

void
Main::init_video()
{
  // FIXME: Add something here
  SCREEN_WIDTH  = 800;
  SCREEN_HEIGHT = 600;

  context_pointer->init_renderer();
  g_screen = SDL_GetVideoSurface();

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
  else {
    log_warning << "Couldn't load icon '" << icon_fname << "'" << std::endl;
  }

  SDL_ShowCursor(0);

  log_info << (g_config->use_fullscreen?"fullscreen ":"window ")
           << " Window: "     << g_config->window_size
           << " Fullscreen: " << g_config->fullscreen_size
           << " Area: "       << g_config->aspect_size << std::endl;
}

void
Main::init_audio()
{
  sound_manager = new SoundManager();

  sound_manager->enable_sound(g_config->sound_enabled);
  sound_manager->enable_music(g_config->music_enabled);
}

void
Main::quit_audio()
{
  if(sound_manager != NULL) {
    delete sound_manager;
    sound_manager = NULL;
  }
}

void
Main::wait_for_event(float min_delay, float max_delay)
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
          g_screen_manager->quit();
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

int
Main::run(int argc, char** argv)
{
  int result = 0;

  try {
    /* Do this before pre_parse_commandline, because --help now shows the
     * default user data dir. */
    init_physfs(argv[0]);

    if(pre_parse_commandline(argc, argv))
      return 0;

    Console::instance = new Console();
    init_sdl();

    timelog("controller");
    g_jk_controller = new JoystickKeyboardController();

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
    scripting::init_squirrel(g_config->enable_script_debugger);

    timelog("resources");
    Resources::load_shared();

    timelog(0);

    const std::auto_ptr<PlayerStatus> default_playerstatus(new PlayerStatus());

    g_screen_manager = new ScreenManager();

    init_rand();

    if(g_config->start_level != "") {
      // we have a normal path specified at commandline, not a physfs path.
      // So we simply mount that path here...
      std::string dir = FileSystem::dirname(g_config->start_level);
      log_debug << "Adding dir: " << dir << std::endl;
      PHYSFS_addToSearchPath(dir.c_str(), true);

      if(g_config->start_level.size() > 4 &&
         g_config->start_level.compare(g_config->start_level.size() - 5, 5, ".stwm") == 0) {
        g_screen_manager->push_screen(new worldmap::WorldMap(
                                 FileSystem::basename(g_config->start_level), default_playerstatus.get()));
      } else {
        std::auto_ptr<GameSession> session (
          new GameSession(FileSystem::basename(g_config->start_level), default_playerstatus.get()));

        g_config->random_seed =session->get_demo_random_seed(g_config->start_demo);
        init_rand();//initialise generator with seed from session

        if(g_config->start_demo != "")
          session->play_demo(g_config->start_demo);

        if(g_config->record_demo != "")
          session->record_demo(g_config->record_demo);
        g_screen_manager->push_screen(session.release());
      }
    } else {
      g_screen_manager->push_screen(new TitleScreen(default_playerstatus.get()));
    }

    g_screen_manager->run(context);
  } catch(std::exception& e) {
    log_fatal << "Unexpected exception: " << e.what() << std::endl;
    result = 1;
  } catch(...) {
    log_fatal << "Unexpected exception" << std::endl;
    result = 1;
  }

  delete g_screen_manager;
  g_screen_manager = NULL;

  Resources::unload_shared();
  quit_audio();

  if(g_config)
    g_config->save();
  delete g_config;
  g_config = NULL;
  delete g_jk_controller;
  g_jk_controller = NULL;
  delete Console::instance;
  Console::instance = NULL;
  scripting::exit_squirrel();
  delete texture_manager;
  texture_manager = NULL;
  SDL_Quit();
  PHYSFS_deinit();

  return result;
}

/* EOF */
