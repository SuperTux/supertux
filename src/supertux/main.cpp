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

#include "supertux/main.hpp"

#include <config.h>
#include <version.h>

#include <SDL_image.h>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <physfs.h>
#include <stdio.h>
#include <tinygettext/log.hpp>
extern "C" {
#include <findlocale.h>
}

#include "addon/addon_manager.hpp"
#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "math/random_generator.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/physfs_file_system.hpp"
#include "physfs/physfs_sdl.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/scripting.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/command_line_arguments.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/title_screen.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "worldmap/worldmap.hpp"

class ConfigSubsystem
{
public:
  ConfigSubsystem()
  {
    g_config.reset(new Config);
    try {
      g_config->load();
    }
    catch(const std::exception& e)
    {
      log_info << "Couldn't load config file: " << e.what() << ", using default settings" << std::endl;
    }

    // init random number stuff
    g_config->random_seed = gameRandom.srand(g_config->random_seed);
    graphicsRandom.srand(0);
    //const char *how = config->random_seed? ", user fixed.": ", from time().";
    //log_info << "Using random seed " << config->random_seed << how << std::endl;
  }

  ~ConfigSubsystem()
  {
    if (g_config)
    {
      g_config->save();
    }
    g_config.reset();
  }
};

void
Main::init_tinygettext()
{
  g_dictionary_manager.reset(new tinygettext::DictionaryManager);
  tinygettext::Log::set_log_info_callback(0);
  g_dictionary_manager->set_filesystem(std::unique_ptr<tinygettext::FileSystem>(new PhysFSFileSystem));

  g_dictionary_manager->add_directory("locale");
  g_dictionary_manager->set_charset("UTF-8");

  // Config setting "locale" overrides language detection
  if (g_config->locale != "")
  {
    g_dictionary_manager->set_language(tinygettext::Language::from_name(g_config->locale));
  }
  else
  {
    FL_Locale *locale;
    FL_FindLocale(&locale);
    tinygettext::Language language = tinygettext::Language::from_spec( locale->lang?locale->lang:"", locale->country?locale->country:"", locale->variant?locale->variant:"");
    FL_FreeLocale(&locale);
    g_dictionary_manager->set_language(language);
  }
}

class PhysfsSubsystem
{
private:
  boost::optional<std::string> m_forced_datadir;
  boost::optional<std::string> m_forced_userdir;

public:
  PhysfsSubsystem(const char* argv0,
                  boost::optional<std::string> forced_datadir,
                  boost::optional<std::string> forced_userdir) :
    m_forced_datadir(forced_datadir),
    m_forced_userdir(forced_userdir)
  {
    if (!PHYSFS_init(argv0))
    {
      std::stringstream msg;
      msg << "Couldn't initialize physfs: " << PHYSFS_getLastError();
      throw std::runtime_error(msg.str());
    }
    else
    {
      // allow symbolic links
      PHYSFS_permitSymbolicLinks(1);

      find_userdir();
      find_datadir();
    }
  }

  void find_datadir()
  {
    std::string datadir;
    if (m_forced_datadir)
    {
      datadir = *m_forced_datadir;
    }
    else if (const char* env_datadir = getenv("SUPERTUX2_DATA_DIR"))
    {
      datadir = env_datadir;
    }
    else
    {
      // check if we run from source dir
      char* basepath_c = SDL_GetBasePath();
      std::string basepath = basepath_c;
      SDL_free(basepath_c);

      datadir = FileSystem::join(basepath, "data");
      std::string testfname = FileSystem::join(datadir, "credits.txt");
      if (!FileSystem::exists(testfname))
      {
        // if the game is not run from the source directory, try to find
        // the global install location
        datadir = datadir.substr(0, datadir.rfind(INSTALL_SUBDIR_BIN));
        datadir = FileSystem::join(datadir, INSTALL_SUBDIR_SHARE);
      }
    }

    if (!PHYSFS_addToSearchPath(datadir.c_str(), 1))
    {
      log_warning << "Couldn't add '" << datadir << "' to physfs searchpath: " << PHYSFS_getLastError() << std::endl;
    }
  }

  void find_userdir()
  {
    std::string userdir;
    if (m_forced_userdir)
    {
      userdir = *m_forced_userdir;
    }
    else if (const char* env_userdir = getenv("SUPERTUX2_USER_DIR"))
    {
      userdir = env_userdir;
    }
    else
    {
      std::string physfs_userdir = PHYSFS_getUserDir();
#ifdef _WIN32
      userdir = FileSystem::join(physfs_userdir, PACKAGE_NAME);
#else
      userdir = FileSystem::join(physfs_userdir, "." PACKAGE_NAME);
#endif
    }

    if (!FileSystem::is_directory(userdir))
    {
      FileSystem::mkdir(userdir);
      log_info << "Created SuperTux userdir: " << userdir << std::endl;
    }

    if (!PHYSFS_setWriteDir(userdir.c_str()))
    {
      std::ostringstream msg;
      msg << "Failed to use userdir directory '"
          <<  userdir << "': " << PHYSFS_getLastError();
      throw std::runtime_error(msg.str());
    }

    PHYSFS_addToSearchPath(userdir.c_str(), 0);
  }

  void print_search_path()
  {
    const char* writedir = PHYSFS_getWriteDir();
    log_info << "PhysfsWritedDir: " << (writedir ? writedir : "(null)") << std::endl;
    log_info << "PhysfsSearchPath:" << std::endl;
    char** searchpath = PHYSFS_getSearchPath();
    for(char** i = searchpath; *i != NULL; ++i)
    {
      log_info << "  " << *i << std::endl;
    }
    PHYSFS_freeList(searchpath);
  }

  ~PhysfsSubsystem()
  {
    PHYSFS_deinit();
  }
};

class SDLSubsystem
{
public:
  SDLSubsystem()
  {
    if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
    {
      std::stringstream msg;
      msg << "Couldn't initialize SDL: " << SDL_GetError();
      throw std::runtime_error(msg.str());
    }
    // just to be sure
    atexit(SDL_Quit);
  }

  ~SDLSubsystem()
  {
    SDL_Quit();
  }
};

void
Main::init_video()
{
  SDL_SetWindowTitle(VideoSystem::current()->get_renderer().get_window(), PACKAGE_NAME " " PACKAGE_VERSION);

  const char* icon_fname = "images/engine/icons/supertux-256x256.png";
  SDL_Surface* icon = IMG_Load_RW(get_physfs_SDLRWops(icon_fname), true);
  if (!icon)
  {
    log_warning << "Couldn't load icon '" << icon_fname << "': " << SDL_GetError() << std::endl;
  }
  else
  {
    SDL_SetWindowIcon(VideoSystem::current()->get_renderer().get_window(), icon);
    SDL_FreeSurface(icon);
  }
  SDL_ShowCursor(0);

  log_info << (g_config->use_fullscreen?"fullscreen ":"window ")
           << " Window: "     << g_config->window_size
           << " Fullscreen: " << g_config->fullscreen_size << "@" << g_config->fullscreen_refresh_rate
           << " Area: "       << g_config->aspect_size << std::endl;
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

void
Main::launch_game()
{
  SDLSubsystem sdl_subsystem;
  ConsoleBuffer console_buffer;

  timelog("controller");
  InputManager input_manager(g_config->keyboard_config, g_config->joystick_config);

  timelog("commandline");

  timelog("video");
  std::unique_ptr<VideoSystem> video_system = VideoSystem::create(g_config->video);
  DrawingContext context(video_system->get_renderer(),
                         video_system->get_lightmap());
  init_video();

  timelog("audio");
  SoundManager sound_manager;
  sound_manager.enable_sound(g_config->sound_enabled);
  sound_manager.enable_music(g_config->music_enabled);

  Console console(console_buffer);

  timelog("scripting");
  scripting::Scripting scripting(g_config->enable_script_debugger);

  timelog("resources");
  TileManager tile_manager;
  SpriteManager sprite_manager;
  Resources resources;

  timelog("addons");
  AddonManager addon_manager(g_config->disabled_addon_filenames);
  addon_manager.load_addons();

  timelog(0);

  const std::unique_ptr<Savegame> default_savegame(new Savegame(std::string()));

  GameManager game_manager;
  ScreenManager screen_manager;

  if(g_config->start_level != "") {
    // we have a normal path specified at commandline, not a physfs path.
    // So we simply mount that path here...
    std::string dir = FileSystem::dirname(g_config->start_level);
    std::string fileProtocol = "file://";
    std::string::size_type position = dir.find(fileProtocol);
    if(position != std::string::npos) {
      dir = dir.replace(position, fileProtocol.length(), "");
    }
    log_debug << "Adding dir: " << dir << std::endl;
    PHYSFS_addToSearchPath(dir.c_str(), true);

    if(g_config->start_level.size() > 4 &&
       g_config->start_level.compare(g_config->start_level.size() - 5, 5, ".stwm") == 0)
    {
      screen_manager.push_screen(std::unique_ptr<Screen>(
                                              new worldmap::WorldMap(
                                                FileSystem::basename(g_config->start_level), *default_savegame)));
    } else {
      std::unique_ptr<GameSession> session (
        new GameSession(FileSystem::basename(g_config->start_level), *default_savegame));

      g_config->random_seed = session->get_demo_random_seed(g_config->start_demo);
      g_config->random_seed = gameRandom.srand(g_config->random_seed);
      graphicsRandom.srand(0);

      if(g_config->start_demo != "")
        session->play_demo(g_config->start_demo);

      if(g_config->record_demo != "")
        session->record_demo(g_config->record_demo);
      screen_manager.push_screen(std::move(session));
    }
  } else {
    screen_manager.push_screen(std::unique_ptr<Screen>(new TitleScreen(*default_savegame)));
  }

  screen_manager.run(context);
}

int
Main::run(int argc, char** argv)
{
  int result = 0;

  try
  {
    CommandLineArguments args;

    try
    {
      args.parse_args(argc, argv);
      g_log_level = args.get_log_level();
    }
    catch(const std::exception& err)
    {
      std::cout << "Error: " << err.what() << std::endl;
      return EXIT_FAILURE;
    }

    PhysfsSubsystem physfs_subsystem(argv[0], args.datadir, args.userdir);
    physfs_subsystem.print_search_path();

    timelog("config");
    ConfigSubsystem config_subsystem;
    args.merge_into(*g_config);

    timelog("tinygettext");
    init_tinygettext();

    switch (args.get_action())
    {
      case CommandLineArguments::PRINT_VERSION:
        args.print_version();
        return 0;

      case CommandLineArguments::PRINT_HELP:
        args.print_help(argv[0]);
        return 0;

      case CommandLineArguments::PRINT_DATADIR:
        args.print_datadir();
        return 0;

      default:
        launch_game();
        break;
    }
  }
  catch(const std::exception& e)
  {
    log_fatal << "Unexpected exception: " << e.what() << std::endl;
    result = 1;
  }
  catch(...)
  {
    log_fatal << "Unexpected exception" << std::endl;
    result = 1;
  }

  g_dictionary_manager.reset();

  return result;
}

/* EOF */
