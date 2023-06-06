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
#include <filesystem>
#include <fstream>

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <physfs.h>
#include <tinygettext/log.hpp>
#include <fmt/format.h>
extern "C" {
#include <findlocale.h>
}

#ifdef WIN32
#include <codecvt>
#endif

#include "addon/addon_manager.hpp"
#include "addon/downloader.hpp"
#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "editor/layer_icon.hpp"
#include "editor/object_info.hpp"
#include "editor/tile_selection.hpp"
#include "editor/tip.hpp"
#include "editor/tool_icon.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "gui/notification.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "object/spawnpoint.hpp"
#include "physfs/physfs_file_system.hpp"
#include "physfs/physfs_sdl.hpp"
#include "port/emscripten.hpp"
#include "sdk/integration.hpp"
#include "sprite/sprite_data.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/command_line_arguments.hpp"
#include "supertux/console.hpp"
#include "supertux/error_handler.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "supertux/menu/download_dialog.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/string_util.hpp"
#include "util/timelog.hpp"
#include "util/string_util.hpp"
#include "video/sdl_surface.hpp"
#include "video/sdl_surface_ptr.hpp"
#include "video/ttf_surface_manager.hpp"
#include "worldmap/worldmap.hpp"
#include "worldmap/worldmap_screen.hpp"

static Timelog s_timelog;

ConfigSubsystem::ConfigSubsystem() :
  m_config()
{
  g_config = &m_config;
  try {
    m_config.load();
  }
  catch(const std::exception& e)
  {
    log_info << "Couldn't load config file: " << e.what() << ", using default settings" << std::endl;
  }

  // init random number stuff
  gameRandom.seed(m_config.random_seed);
  graphicsRandom.seed(0);
  //const char *how = config->random_seed? ", user fixed.": ", from time().";
  //log_info << "Using random seed " << config->random_seed << how << std::endl;
}

ConfigSubsystem::~ConfigSubsystem()
{
  try
  {
    m_config.save();
  }
  catch(std::exception& e)
  {
    log_warning << "Error saving config: " << e.what() << std::endl;
  }
}

Main::Main() :
  m_physfs_subsystem(),
  m_config_subsystem(),
  m_sdl_subsystem(),
  m_console_buffer(),
  m_input_manager(),
  m_video_system(),
  m_ttf_surface_manager(),
  m_sound_manager(),
  m_squirrel_virtual_machine(),
  m_tile_manager(),
  m_sprite_manager(),
  m_resources(),
  m_addon_manager(),
  m_console(),
  m_game_manager(),
  m_screen_manager(),
  m_savegame(),
  m_downloader() // Used for getting the version of the latest SuperTux release.
{
}

void
Main::init_tinygettext()
{
  g_dictionary_manager.reset(new tinygettext::DictionaryManager(std::make_unique<PhysFSFileSystem>(), "UTF-8"));

  tinygettext::Log::set_log_info_callback(log_info_callback);
  tinygettext::Log::set_log_warning_callback(log_warning_callback);
  tinygettext::Log::set_log_error_callback(log_error_callback);

  g_dictionary_manager->add_directory("locale");

  // Config setting "locale" overrides language detection
  if (!g_config->locale.empty())
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

PhysfsSubsystem::PhysfsSubsystem(const char* argv0,
                std::optional<std::string> forced_datadir,
                std::optional<std::string> forced_userdir) :
  m_forced_datadir(std::move(forced_datadir)),
  m_forced_userdir(std::move(forced_userdir))
{
  if (!PHYSFS_init(argv0))
  {
    std::stringstream msg;
    msg << "Couldn't initialize physfs: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
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

void PhysfsSubsystem::find_datadir() const
{
#ifndef __EMSCRIPTEN__
#ifdef __ANDROID__
    if (!setup_android_datadir())
    {
      log_warning << "Couldn't setup android assets: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
    }
    return;
#endif

  std::string datadir;
  if (m_forced_datadir)
  {
    datadir = *m_forced_datadir;
  }
  else if (const char* env_datadir = getenv("SUPERTUX2_DATA_DIR"))
  {
    datadir = env_datadir;
  }
  else if (const char* env_datadir3 = getenv("ANDROID_MY_OWN_APP_FILE"))
  {
    datadir = env_datadir3;
  }
  else
  {
    // check if we run from source dir
    char* basepath_c = SDL_GetBasePath();
    std::string basepath = basepath_c ? basepath_c : "./";
    SDL_free(basepath_c);

    if (FileSystem::exists(FileSystem::join(BUILD_DATA_DIR, "credits.stxt")))
    {
      datadir = BUILD_DATA_DIR;
      // Add config dir for supplemental files
      PHYSFS_mount(std::filesystem::canonical(BUILD_CONFIG_DATA_DIR).string().c_str(), nullptr, 1);
    }
    else
    {
      // if the game is not run from the source directory, try to find
      // the global install location
      datadir = basepath.substr(0, basepath.rfind(INSTALL_SUBDIR_BIN));
      datadir = FileSystem::join(datadir, INSTALL_SUBDIR_SHARE);
    }
  }

  if (!PHYSFS_mount(std::filesystem::canonical(datadir).string().c_str(), nullptr, 1))
  {
    log_warning << "Couldn't add '" << datadir << "' to physfs searchpath: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
  }
#else
  if (!PHYSFS_mount(BUILD_CONFIG_DATA_DIR, nullptr, 1))
  {
    log_warning << "Couldn't add '" << BUILD_CONFIG_DATA_DIR << "' to physfs searchpath: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
  }
#endif
}

void PhysfsSubsystem::find_userdir() const
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
  userdir = PHYSFS_getPrefDir("SuperTux","supertux2");
  }
//Kept for backwards-compatability only, hence the silence
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
std::string physfs_userdir = PHYSFS_getUserDir();
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifndef __HAIKU__
#ifdef _WIN32
std::string olduserdir = FileSystem::join(physfs_userdir, PACKAGE_NAME);
#else
std::string olduserdir = FileSystem::join(physfs_userdir, "." PACKAGE_NAME);
#endif
if (FileSystem::is_directory(olduserdir)) {
  std::filesystem::path olduserpath(olduserdir);
  std::filesystem::path userpath(userdir);

  std::filesystem::directory_iterator end_itr;

  bool success = true;

  // cycle through the directory
  for (std::filesystem::directory_iterator itr(olduserpath); itr != end_itr; ++itr) {
  try
  {
    std::filesystem::rename(itr->path().string().c_str(), userpath / itr->path().filename());
  }
  catch (const std::filesystem::filesystem_error& err)
  {
    success = false;
    log_warning << "Failed to move contents of config directory: " << err.what() << std::endl;
  }
  }
  if (success) {
    try
    {
      std::filesystem::remove_all(olduserpath);
    }
    catch (const std::filesystem::filesystem_error& err)
    {
      success = false;
      log_warning << "Failed to remove old config directory: " << err.what();
    }
  }
  if (success) {
    log_info << "Moved old config dir " << olduserdir << " to " << userdir << std::endl;
  }
}
#endif

#ifdef EMSCRIPTEN
  userdir = "/home/web_user/.local/share/supertux2/";
#endif

  if (!FileSystem::is_directory(userdir))
  {
  FileSystem::mkdir(userdir);
  log_info << "Created SuperTux userdir: " << userdir << std::endl;
  }

#ifdef EMSCRIPTEN
  EM_ASM({
    try {
      FS.mount(IDBFS, {}, "/home/web_user/.local/share/supertux2/");
      FS.syncfs(true, (err) => { console.log(err); });
    } catch(err) {}
  }, 0); // EM_ASM is a variadic macro and Clang requires at least 1 value for the variadic argument
#endif

  if (!PHYSFS_setWriteDir(userdir.c_str()))
  {
    std::ostringstream msg;
    msg << "Failed to use userdir directory '"
        <<  userdir << "': errorcode: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    throw std::runtime_error(msg.str());
  }

  PHYSFS_mount(userdir.c_str(), nullptr, 0);
}

void PhysfsSubsystem::print_search_path()
{
  const char* writedir = PHYSFS_getWriteDir();
  log_info << "PhysfsWriteDir: " << (writedir ? writedir : "(null)") << std::endl;
  log_info << "PhysfsSearchPath:" << std::endl;
  char** searchpath = PHYSFS_getSearchPath();
  for (char** i = searchpath; *i != nullptr; ++i)
  {
    log_info << "  " << *i << std::endl;
  }
  PHYSFS_freeList(searchpath);
}

bool
PhysfsSubsystem::setup_android_datadir() const
{
  /*
  Only SDL knows about the data zip inside the APK
  (this means only SDL knows how to use the JNI to load assets)
  1. Copy the data zip inside the apk to userdir
  2. Physfs mounts that new data zip
  This solution works... until it doesn't
  */

  std::string zippath = "data.zip";
  std::string newzip = m_forced_userdir.value();
  newzip.append("/");
  newzip.append(zippath);

  size_t zipsz;
  void* zipdata = SDL_LoadFile(zippath.c_str(), &zipsz);

  bool newdata = true;
  if (FileSystem::exists(newzip)) {
    // Oh wait, the zip already exists?
    // Well is it different?
    size_t currzipsz;
    void* currzip = SDL_LoadFile(newzip.c_str(), &currzipsz);

    newdata = (zipsz != currzipsz);

    SDL_free(currzip);
  }

  if (newdata) {
    // Copy
    SDL_RWops* zipcp = SDL_RWFromFile(newzip.c_str(), "w");
    if (!zipcp) return false;
    SDL_RWwrite(zipcp, zipdata, sizeof(char), zipsz);
    SDL_RWclose(zipcp);
  }

  SDL_free(zipdata);

  PHYSFS_mount(newzip.c_str(), NULL, 1);

  return true;
}

PhysfsSubsystem::~PhysfsSubsystem()
{
  PHYSFS_deinit();
}

SDLSubsystem::SDLSubsystem()
{
  Uint32 flags = SDL_INIT_TIMER | SDL_INIT_VIDEO;
#ifndef UBUNTU_TOUCH
  flags |= SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;
#endif
  if (SDL_Init(flags) < 0)
  {
    std::stringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  if (TTF_Init() < 0)
  {
    std::stringstream msg;
    msg << "Couldn't initialize SDL TTF: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  // just to be sure
  atexit(TTF_Quit);
  atexit(SDL_Quit);
}

SDLSubsystem::~SDLSubsystem()
{
  TTF_Quit();
  SDL_Quit();
}

void
Main::init_video()
{
  VideoSystem::current()->set_title("SuperTux " PACKAGE_VERSION);

  const char* icon_fname = "images/engine/icons/supertux-256x256.png";

  SDLSurfacePtr icon = SDLSurface::from_file(icon_fname);
  VideoSystem::current()->set_icon(*icon);

  SDL_ShowCursor(g_config->custom_mouse_cursor ? 0 : 1);

  log_info << (g_config->use_fullscreen?"fullscreen ":"window ")
           << " Window: "     << g_config->window_size
           << " Fullscreen: " << g_config->fullscreen_size << "@" << g_config->fullscreen_refresh_rate
           << " Area: "       << g_config->aspect_size << std::endl;
}

void
Main::resave(const std::string& input_filename, const std::string& output_filename)
{
  Editor::s_resaving_in_progress = true;
  std::ifstream in(input_filename);
  if (!in) {
    log_fatal << input_filename << ": couldn't open file for reading" << std::endl;
  } else {
    log_info << "loading level: " << input_filename << std::endl;
    auto level = LevelParser::from_stream(in, input_filename, StringUtil::has_suffix(input_filename, ".stwm"), true);
    in.close();

    std::ofstream out(output_filename);
    if (!out) {
      log_fatal << output_filename << ": couldn't open file for writing" << std::endl;
    } else {
      log_info << "saving level: " << output_filename << std::endl;
      level->save(out);
    }
  }
  Editor::s_resaving_in_progress = false;
}

void
Main::launch_game(const CommandLineArguments& args)
{
  //m_sdl_subsystem.reset(new SDLSubsystem());
  m_console_buffer.reset(new ConsoleBuffer());
#ifdef ENABLE_TOUCHSCREEN_SUPPORT
  if (getenv("ANDROID_TV")) {
    g_config->mobile_controls = false;
  }
#endif

  s_timelog.log("controller");
  m_input_manager.reset(new InputManager(g_config->keyboard_config, g_config->joystick_config));

  s_timelog.log("commandline");

#ifndef EMSCRIPTEN
  auto video = g_config->video;
  if (args.resave && *args.resave) {
    if (args.video) {
      video = *args.video;
    } else {
      video = VideoSystem::VIDEO_NULL;
    }
  }
  s_timelog.log("video");

  m_video_system = VideoSystem::create(video);
#else
  // Force SDL for WASM builds, as OpenGL is reportedly slow on some devices
  m_video_system = VideoSystem::create(VideoSystem::VIDEO_SDL);
#endif
  init_video();

  m_ttf_surface_manager.reset(new TTFSurfaceManager());

  s_timelog.log("audio");
  m_sound_manager.reset(new SoundManager());
  m_sound_manager->enable_sound(g_config->sound_enabled);
  m_sound_manager->enable_music(g_config->music_enabled);
  m_sound_manager->set_sound_volume(g_config->sound_volume);
  m_sound_manager->set_music_volume(g_config->music_volume);

  s_timelog.log("scripting");
  m_squirrel_virtual_machine.reset(new SquirrelVirtualMachine(g_config->enable_script_debugger));

  s_timelog.log("resources");
  m_tile_manager.reset(new TileManager());
  m_sprite_manager.reset(new SpriteManager());
  m_resources.reset(new Resources());

  s_timelog.log("integrations");
  Integration::setup();

  s_timelog.log("addons");
  m_addon_manager.reset(new AddonManager("addons", g_config->addons));

  m_console.reset(new Console(*m_console_buffer));

  s_timelog.log(nullptr);

  m_savegame = std::make_unique<Savegame>(std::string());

  m_game_manager.reset(new GameManager());
  m_screen_manager.reset(new ScreenManager(*m_video_system, *m_input_manager));

  if (!args.filenames.empty())
  {
    for(const auto& start_level : args.filenames)
    {
      // we have a normal path specified at commandline, not a physfs path.
      // So we simply mount that path here...
      std::string dir = FileSystem::dirname(start_level);
      const std::string filename = FileSystem::basename(start_level);
      const std::string fileProtocol = "file://";
      const std::string::size_type position = dir.find(fileProtocol);
      if (position != std::string::npos) {
        dir = dir.replace(position, fileProtocol.length(), "");
      }
      log_debug << "Adding dir: " << dir << std::endl;
      PHYSFS_mount(dir.c_str(), nullptr, true);

      if (args.resave && *args.resave)
      {
        resave(start_level, start_level);
      }
      else if (args.editor)
      {
        if (PHYSFS_exists(start_level.c_str())) {
          auto editor = std::make_unique<Editor>();
          editor->set_level(start_level);
          editor->setup();
          editor->update(0, Controller());
          m_screen_manager->push_screen(std::move(editor));
          MenuManager::instance().clear_menu_stack();
          m_sound_manager->stop_music(0.5);
        } else {
          log_warning << "Level " << start_level << " doesn't exist." << std::endl;
        }
      }
      else if (StringUtil::has_suffix(start_level, ".stwm"))
      {
        m_screen_manager->push_screen(std::make_unique<worldmap::WorldMapScreen>(
                                     std::make_unique<worldmap::WorldMap>(filename, *m_savegame)));
      }
      else
      { // launch game
        std::unique_ptr<GameSession> session (
          new GameSession(filename, *m_savegame));

        g_config->random_seed = session->get_demo_random_seed(g_config->start_demo);
        gameRandom.seed(g_config->random_seed);
        graphicsRandom.seed(0);

        if (args.sector || args.spawnpoint)
        {
          std::string sectorname = args.sector.value_or("main");

          const auto& spawnpoints = session->get_current_sector().get_objects_by_type<SpawnPointMarker>();
          std::string default_spawnpoint = (spawnpoints.begin() != spawnpoints.end()) ?
            "" : spawnpoints.begin()->get_name();
          std::string spawnpointname = args.spawnpoint.value_or(default_spawnpoint);

          session->set_start_point(sectorname, spawnpointname);
          session->restart_level();
        }

        if (g_config->tux_spawn_pos)
        {
          // FIXME: Specify start pos for multiple players
          session->get_current_sector().get_players()[0]->set_pos(*g_config->tux_spawn_pos);
        }

        if (!g_config->start_demo.empty())
          session->play_demo(g_config->start_demo);

        if (!g_config->record_demo.empty())
          session->record_demo(g_config->record_demo);
        m_screen_manager->push_screen(std::move(session));
      }
    }
  }
  else
  {
    if (args.editor)
    {
      m_screen_manager->push_screen(std::make_unique<Editor>());
    }
    else
    {
      m_screen_manager->push_screen(std::make_unique<TitleScreen>(*m_savegame));
      if (g_config->do_release_check) release_check();
    }
  }

  m_screen_manager->run();
}

int
Main::run(int argc, char** argv)
{
  // First and foremost, set error handlers (to print stack trace on SIGSEGV, etc.)
  ErrorHandler::set_handlers();

#ifdef __EMSCRIPTEN__
  init_emscripten();
#endif

#ifdef WIN32
	//SDL is used instead of PHYSFS because both create the same path in app data
	//However, PHYSFS is not yet initizlized, and this should be run before anything is initialized
	std::string prefpath = SDL_GetPrefPath("SuperTux", "supertux2");

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	//All this conversion stuff is necessary to make this work for internationalized usernames
	std::string outpath = prefpath + u8"/console.out";
	std::wstring w_outpath = converter.from_bytes(outpath);
	_wfreopen(w_outpath.c_str(), L"a", stdout);

	std::string errpath = prefpath + u8"/console.err";
	std::wstring w_errpath = converter.from_bytes(errpath);
	_wfreopen(w_errpath.c_str(), L"a", stderr);

  // Create and install global locale - this can fail on some situations:
  // - with bad values for env vars (LANG, LC_ALL, ...)
  // - targets where libstdc++ uses its generic locales code (https://gcc.gnu.org/legacy-ml/libstdc++/2003-02/msg00345.html)
  try
  {
    std::locale::global(std::locale::classic());
  }
  catch(const std::runtime_error& err)
  {
    std::cout << "Warning: " << err.what() << std::endl;
  }
#endif

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

#ifdef __ANDROID__
    // nasty workaround
    m_physfs_subsystem.reset(new PhysfsSubsystem(nullptr, args.datadir, SDL_AndroidGetExternalStoragePath()));
#else
    m_physfs_subsystem.reset(new PhysfsSubsystem(nullptr, args.datadir, args.userdir));
#endif
    m_physfs_subsystem->print_search_path();

    m_sdl_subsystem.reset(new SDLSubsystem());

    s_timelog.log("config");
    m_config_subsystem.reset(new ConfigSubsystem());
    args.merge_into(*g_config);

    s_timelog.log("tinygettext");
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

      case CommandLineArguments::PRINT_ACKNOWLEDGEMENTS:
        args.print_acknowledgements();
        return 0;

      default:
        launch_game(args);
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

void
Main::release_check()
{
  // Detect a potential new release of SuperTux. If a release, other than
  // the current one is indicated on the given web file, show a notification on the main menu screen.
  const std::string target_file = "ver_info.nfo";
  TransferStatusPtr status = m_downloader.request_download("https://raw.githubusercontent.com/SuperTux/addons/master/ver_info.nfo", target_file);
  status->then([target_file, status](bool success)
  {
    if (!success)
    {
      log_warning << "Error performing new release check: Failed to download \"supertux-versioninfo\" file: " << status->error_msg << std::endl;
      return;
    }
    auto doc = ReaderDocument::from_file(target_file);
    auto root = doc.get_root();
    if (root.get_name() != "supertux-versioninfo")
    {
      log_warning << "File is not a \"supertux-versioninfo\" file." << std::endl;
      return;
    }
    auto mapping = root.get_mapping();
    std::string latest_ver;
    if (mapping.get("latest", latest_ver))
    {
      const std::string version_full = std::string(PACKAGE_VERSION);
      const std::string version = version_full.substr(version_full.find("v") + 1, version_full.find("-") - 1);
      if (version != latest_ver)
      {
        auto notif = std::make_unique<Notification>("new_release_" + latest_ver);
        notif->set_text("New release: SuperTux v" + latest_ver + "!");
        notif->on_press([latest_ver]()
                       {
                         Dialog::show_confirmation(fmt::format(fmt::runtime(_("A new release of SuperTux (v{}) is available!\nFor more information, you can visit the SuperTux website.\n \nDo you want to visit the website now?")), latest_ver), []()
                                                   {
                                                     FileSystem::open_url("https://supertux.org");
                                                   });
                       });
        MenuManager::instance().set_notification(std::move(notif));
      }
    }
  });
  // Set up a download dialog to update the transfer status.
  auto dialog = std::make_unique<DownloadDialog>(status, true, true, true);
  dialog->set_title(_("Checking for new releases..."));
  MenuManager::instance().set_dialog(std::move(dialog));
}

/* EOF */
