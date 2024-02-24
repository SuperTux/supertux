//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "squirrel/supertux_api.hpp"

#include <simplesquirrel/table.hpp>
#include <simplesquirrel/vm.hpp>

#include "audio/sound_manager.hpp"
#include "math/anchor_point.hpp"
#include "math/random.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "physfs/ifile_stream.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "supertux/console.hpp"
#include "supertux/debug.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/textscroller_screen.hpp"
#include "supertux/title_screen.hpp"
#include "worldmap/worldmap.hpp"

namespace scripting {

namespace Globals {

/**
 * Displays the value of an argument. This is useful for inspecting tables.
 * @param ANY $object
 */
void display(const ssq::Object& object)
{
  ConsoleBuffer::output << squirrel_to_string(object) << std::endl;
}
/**
 * Displays the contents of the current stack.
 */
void print_stacktrace(HSQUIRRELVM vm)
{
  print_squirrel_stack(vm);
}
/**
 * Returns the currently running thread.
 */
SQInteger get_current_thread(HSQUIRRELVM vm)
{
  sq_pushthread(vm, vm);
  return 1;
}

/**
 * Returns whether the game is in christmas mode.
 */
bool is_christmas()
{
  return g_config->christmas_mode;
}

/**
 * Starts a skippable cutscene.
 */
void start_cutscene()
{
  auto session = GameSession::current();
  if (session == nullptr)
  {
    log_info << "No game session" << std::endl;
    return;
  }

  if (session->get_current_level().m_is_in_cutscene)
  {
    log_warning << "start_cutscene(): starting a new cutscene above another one, ending preceding cutscene (use end_cutscene() in scripts!)" << std::endl;

    // Remove all sounds that started playing while skipping.
    if (session->get_current_level().m_skip_cutscene)
      SoundManager::current()->stop_sounds();
  }

  session->get_current_level().m_is_in_cutscene = true;
  session->get_current_level().m_skip_cutscene = false;
}
/**
 * Ends a skippable cutscene.
 */
void end_cutscene()
{
  auto session = GameSession::current();
  if (session == nullptr)
  {
    log_info << "No game session." << std::endl;
    return;
  }

  if (!session->get_current_level().m_is_in_cutscene)
  {
    log_warning << "end_cutscene(): no cutscene to end, resetting status anyways." << std::endl;
  }

  // Remove all sounds that started playing while skipping.
  if (session->get_current_level().m_skip_cutscene)
    SoundManager::current()->stop_sounds();

  session->get_current_level().m_is_in_cutscene = false;
  session->get_current_level().m_skip_cutscene = false;
}
/**
 * Checks if a skippable cutscene is currently running.
 */
bool check_cutscene()
{
  auto session = GameSession::current();
  if (session == nullptr)
  {
    log_info << "No game session" << std::endl;
    return false;
  }

  return session->get_current_level().m_is_in_cutscene;
}

/**
 * Suspends the script execution for a specified number of seconds.
 * @param float $seconds
 */
SQInteger wait(HSQUIRRELVM vm, float seconds)
{
  ssq::VM& ssq_vm = ssq::VM::get(vm);
  if (!ssq_vm.isThread()) return 0;

  auto session = GameSession::current();
  if (session && session->get_current_level().m_skip_cutscene)
  {
    if (ssq_vm.getForeignPtr())
    {
      auto squirrelenv = ssq_vm.getForeignPtr<SquirrelEnvironment>();
      // Wait anyways, to prevent scripts like `while (true) {wait(0.1); ...}`.
      return squirrelenv->wait_for_seconds(vm, 0);
    }
    else
    {
      auto squirrelvm = ssq::VM::getMain(vm).getForeignPtr<SquirrelVirtualMachine>();
      return squirrelvm->wait_for_seconds(vm, 0);
    }
  }
  else if (session && session->get_current_level().m_is_in_cutscene)
  {
    if (ssq_vm.getForeignPtr())
    {
      auto squirrelenv = ssq_vm.getForeignPtr<SquirrelEnvironment>();
      // Wait anyways, to prevent scripts like `while (true) {wait(0.1); ...}` from freezing the game.
      return squirrelenv->skippable_wait_for_seconds(vm, seconds);
    }
    else
    {
      auto squirrelvm = ssq::VM::getMain(vm).getForeignPtr<SquirrelVirtualMachine>();
      return squirrelvm->skippable_wait_for_seconds(vm, seconds);
    }
  }
  else
  {
    if (ssq_vm.getForeignPtr())
    {
      auto squirrelenv = ssq_vm.getForeignPtr<SquirrelEnvironment>();
      return squirrelenv->wait_for_seconds(vm, seconds);
    }
    else
    {
      auto squirrelvm = ssq::VM::getMain(vm).getForeignPtr<SquirrelVirtualMachine>();
      return squirrelvm->wait_for_seconds(vm, seconds);
    }
  }
}

/**
 * Suspends the script execution until the current screen has been changed.
 */
SQInteger wait_for_screenswitch(HSQUIRRELVM vm)
{
  auto squirrelvm = ssq::VM::getMain(vm).getForeignPtr<SquirrelVirtualMachine>();
  return squirrelvm->wait_for_screenswitch(vm);
}
/**
 * Exits the currently running screen (for example, force exits from worldmap or scrolling text).
 */
void exit_screen()
{
  ScreenManager::current()->pop_screen();
}

/**
 * Translates a text into the user's language (by looking in the "".po"" files).
 * @param string $text
 */
std::string translate(const std::string& text)
{
  return g_dictionary_manager->get_dictionary().translate(text);
}
#ifdef DOXYGEN_SCRIPTING
/**
 * Same function as ""translate()"".
 * @param string $text
 */
std::string _(const std::string& text)
{
}
#endif
/**
 * Translates a text into the user's language (by looking in the "".po"" files).
   Returns ""text"" or ""text_plural"", depending on ""num"" and the locale.
 * @param string $text
 * @param string $text_plural
 * @param int $num
 */
std::string translate_plural(const std::string& text, const std::string& text_plural, int num)
{
  return g_dictionary_manager->get_dictionary().translate_plural(text, text_plural, num);
}
#ifdef DOXYGEN_SCRIPTING
/**
 * Same function as ""translate_plural()"".
 * @param string $text
 * @param string $text_plural
 * @param int $num
 */
std::string __(const std::string& text, const std::string& text_plural, int num)
{
}
#endif

/**
 * Displays a text file and scrolls it over the screen (on next screenswitch).
 * @param string $filename
 */
void display_text_file(const std::string& filename)
{
  ScreenManager::current()->push_screen(std::make_unique<TextScrollerScreen>(filename));
}

/**
 * Loads and displays a worldmap (on next screenswitch), using the savegame of the current worldmap.
 * @param string $filename
 * @param string $sector Forced sector to spawn in the worldmap on. Leave empty to use last sector from savegame.
 * @param string $spawnpoint Forced spawnpoint to spawn in the worldmap on. Leave empty to use last position from savegame.
 */
void load_worldmap(const std::string& filename, const std::string& sector, const std::string& spawnpoint)
{
  using namespace worldmap;

  if (!WorldMap::current())
  {
    throw std::runtime_error("Can't start Worldmap without active WorldMap");
  }
  else
  {
    WorldMap::current()->change(filename, sector, spawnpoint);
  }
}
/**
 * Switches to a different worldmap after unloading the current one, after ""exit_screen()"" is called.
 * @param string $dirname The world directory, where the "worldmap.stwm" file is located.
 * @param string $sector Forced sector to spawn in the worldmap on. Leave empty to use last sector from savegame.
 * @param string $spawnpoint Forced spawnpoint to spawn in the worldmap on. Leave empty to use last position from savegame.
 */
void set_next_worldmap(const std::string& dirname, const std::string& sector, const std::string& spawnpoint)
{
  GameManager::current()->set_next_worldmap(dirname, sector, spawnpoint);
}
/**
 * Loads and displays a level (on next screenswitch), using the savegame of the current level.
 * @param string $filename
 */
void load_level(const std::string& filename)
{
  if (!GameSession::current())
  {
    throw std::runtime_error("Can't start level without active level.");
  }
  else
  {
    ScreenManager::current()->push_screen(std::make_unique<GameSession>(filename, GameSession::current()->get_savegame()));
  }
}

/**
 * Loads a script file and executes it. This is typically used to import functions from external files.
 * @param string $filename
 */
void import(HSQUIRRELVM vm, const std::string& filename)
{
  ssq::VM& ssq_vm = ssq::VM::get(vm);

  IFileStream in(filename);
  ssq_vm.run(ssq_vm.compileSource(in, filename.c_str()));
}

/**
 * Enables/disables drawing of collision rectangles.
 * @param bool $enable
 */
void debug_collrects(bool enable)
{
  g_debug.show_collision_rects = enable;
}
/**
 * Enables/disables drawing of FPS.
 * @param bool $enable
 */
void debug_show_fps(bool enable)
{
  g_config->show_fps = enable;
}
/**
 * Enables/disables drawing of non-solid layers.
 * @param bool $enable
 */
void debug_draw_solids_only(bool enable)
{
  ::Sector::s_draw_solids_only = enable;
}
/**
 * Enables/disables drawing of editor images.
 * @param bool $enable
 */
void debug_draw_editor_images(bool enable)
{
  Tile::draw_editor_images = enable;
}
/**
 * Enables/disables worldmap ghost mode.
 * @param bool $enable
 */
void debug_worldmap_ghost(bool enable)
{
  auto worldmap_sector = worldmap::WorldMapSector::current();

  if (worldmap_sector == nullptr)
    throw std::runtime_error("Can't change ghost mode without active WorldMapSector.");

  auto& tux = worldmap_sector->get_singleton_by_type<worldmap::Tux>();
  tux.set_ghost_mode(enable);
}
/**
 * Sets the game speed to ""speed"".
 * @param float $speed
 */
void set_game_speed(float speed)
{
  if (speed < 0.05f)
  {
    // Always put a minimum speed above 0 - if the user enabled transitions,
    // executing transitions would take an unreaonably long time if we allow
    // game speeds like 0.00001.
    log_warning << "Cannot set game speed to less than 0.05" << std::endl;
    throw std::runtime_error("Cannot set game speed to less than 0.05");
  }

  ::g_debug.set_game_speed_multiplier(speed);
}

/**
 * Saves world state to scripting table.
 */
void save_state()
{
  auto worldmap = worldmap::WorldMap::current();

  if (!worldmap)
  {
    throw std::runtime_error("Can't save state without active Worldmap.");
  }
  else
  {
    worldmap->save_state();
  }
}
/**
 * Loads world state from scripting table.
 */
void load_state()
{
  auto worldmap = worldmap::WorldMap::current();

  if (!worldmap)
  {
    throw std::runtime_error("Can't save state without active Worldmap.");
  }
  else
  {
    worldmap->load_state();
  }
}

/**
 * Changes the music to ""musicfile"".
 * @param string $musicfile
 */
void play_music(const std::string& filename)
{
  SoundManager::current()->play_music(filename);
}
/**
 * Fades in the music from ""musicfile"" for ""fadetime"" seconds.
 * @param string $musicfile
 * @param float $fadetime
 */
void fade_in_music(const std::string& filename, float fadetime)
{
  SoundManager::current()->play_music(filename, fadetime);
}
/**
 * Fades out the music for ""fadetime"" seconds.
 * @param float $fadetime Set to "0" for no fade-out.
 */
void stop_music(float fadetime)
{
  SoundManager::current()->stop_music(fadetime);
}
/**
 * Resumes and fades in the music for ""fadetime"" seconds.
 * @param float $fadetime Set to "0" for no fade-in.
 */
void resume_music(float fadetime)
{
  SoundManager::current()->resume_music(fadetime);
}
/**
 * Pauses the music with a fade-out for ""fadetime"" seconds.
 * @param float $fadetime Set to "0" for no fade-out.
 */
void pause_music(float fadetime)
{
  SoundManager::current()->pause_music(fadetime);
}
/**
 * Plays ""soundfile"" as a sound.
 * @param string $soundfile
 */
void play_sound(const std::string& filename)
{
  SoundManager::current()->play(filename);
}

/**
 * Speeds Tux up.
 */
void grease()
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]); // scripting::Player != ::Player
  tux.get_physic().set_velocity_x(tux.get_physic().get_velocity_x()*3);
}
/**
 * Makes Tux invincible for 10000 units of time.
 */
void invincible()
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.m_invincible_timer.start(10000);
}
/**
 * Makes Tux a ghost, i.e. lets him float around and through solid objects.
 */
void ghost()
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.set_ghost_mode(true);
}
/**
 * Recalls Tux's invincibility and ghost status.
 */
void mortal()
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.m_invincible_timer.stop();
  tux.set_ghost_mode(false);
}
/**
 * Re-initializes and respawns Tux at the beginning of the current level.
 */
void restart()
{
  auto session = GameSession::current();
  if (session == nullptr)
  {
    log_info << "No game session." << std::endl;
    return;
  }
  session->reset_button = true;
}
/**
 * Prints Tux's current coordinates in the current level.
 */
void whereami()
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  log_info << "You are at x " << (static_cast<int>(tux.get_pos().x)) << ", y " << (static_cast<int>(tux.get_pos().y)) << std::endl;
}
/**
 * Moves Tux near the end of the current level.
 */
void gotoend()
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.set_pos(Vector((::Sector::get().get_width()) - (static_cast<float>(SCREEN_WIDTH) * 2.0f), 0));
  ::Sector::get().get_camera().reset(
    Vector(tux.get_pos().x, tux.get_pos().y));
}
/**
 * Moves Tux to the X and Y blocks, relative to his position.
 * @param float $offset_x
 * @param float $offset_y
 */
void warp(float offset_x, float offset_y)
{
  if (!Sector::current()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.set_pos(Vector(tux.get_pos().x + (offset_x*32), tux.get_pos().y - (offset_y*32)));
  ::Sector::get().get_camera().reset(
    Vector(tux.get_pos().x, tux.get_pos().y));
}

/**
 * Adjusts the gamma.
 * @param float $gamma
 */
void set_gamma(float gamma)
{
  VideoSystem::current()->set_gamma(gamma);
}

/**
 * Returns a random integer.
 */
int rand()
{
  return gameRandom.rand();
}

/**
 * Sets the frame, displayed on the title screen.
 * @param string $image
 */
void set_title_frame(const std::string& image)
{
  auto title_screen = TitleScreen::current();
  if (!title_screen)
  {
    log_info << "No title screen loaded." << std::endl;
    return;
  }
  title_screen->set_frame(image);
}

} // namespace Globals


namespace Level {

/**
 * Ends the current level.
 * @param bool $win If ""true"", the level is marked as completed if launched from a worldmap.
 */
void finish(bool win)
{
  if (!GameSession::current()) return;
  GameSession::current()->finish(win);
}
/**
 * Returns whether an end sequence has started. (AKA when the stats at the end are visible)
 */
bool has_active_sequence()
{
  if (!GameSession::current()) return false;
  return GameSession::current()->has_active_sequence();
}

/**
 * Respawns Tux in sector named ""sector"" at spawnpoint named ""spawnpoint"".${SRG_TABLENEWPARAGRAPH}
   Exceptions: If ""sector"" or ""spawnpoint"" are empty, or the specified sector does not exist, the function will bail out the first chance it gets.
   If the specified spawnpoint doesn't exist, Tux will be spawned at the spawnpoint named “main”.
   If that spawnpoint doesn't exist either, Tux will simply end up at the origin (top-left 0, 0).
 * @param string $sector
 * @param string $spawnpoint
 */
void spawn(const std::string& sector, const std::string& spawnpoint)
{
  if (!GameSession::current()) return;
  GameSession::current()->respawn(sector, spawnpoint);
}

/**
 * Sets the default start spawnpoint of the level.
 * @param string $sector
 * @param string $spawnpoint
 */
void set_start_point(const std::string& sector, const std::string& spawnpoint)
{
  if (!GameSession::current()) return;
  GameSession::current()->set_start_point(sector, spawnpoint);
}
/**
 * Sets the default start spawn position of the level.
 * @param string $sector
 * @param float $x
 * @param float $y
 */
void set_start_pos(const std::string& sector, float x, float y)
{
  if (!GameSession::current()) return;
  GameSession::current()->set_start_pos(sector, Vector(x, y));
}
/**
 * Sets the default respawn spawnpoint of the level.
 * @param string $sector
 * @param string $spawnpoint
 */
void set_respawn_point(const std::string& sector, const std::string& spawnpoint)
{
  if (!GameSession::current()) return;
  GameSession::current()->set_respawn_point(sector, spawnpoint);
}
/**
 * Sets the default respawn position of the level.
 * @param string $sector
 * @param float $x
 * @param float $y
 */
void set_respawn_pos(const std::string& sector, float x, float y)
{
  if (!GameSession::current()) return;
  GameSession::current()->set_respawn_pos(sector, Vector(x, y));
}

/**
 * Flips the level vertically (i.e. top is now bottom and vice versa).
   Call again to revert the effect. Make sure the player can land on something after the level is flipped!
 */
void flip_vertically()
{
  if (!GameSession::current()) return;
  BIND_SECTOR(::Sector::get());
  FlipLevelTransformer flip_transformer;
  flip_transformer.transform(GameSession::current()->get_current_level());
}

/**
 * Toggles pause.
 */
void toggle_pause()
{
  if (!GameSession::current()) return;
  GameSession::current()->toggle_pause();
}

/**
 * Pauses the target timer.
 */
void pause_target_timer()
{
  if (!GameSession::current()) return;
  GameSession::current()->set_target_timer_paused(true);
}
/**
 * Resumes the target timer.
 */
void resume_target_timer()
{
  if (!GameSession::current()) return;
  GameSession::current()->set_target_timer_paused(false);
}

} // namespace Level

} // namespace scripting


void register_supertux_scripting_api(ssq::VM& vm)
{
  /* GameObject-s */
  GameObjectFactory::register_objects(vm);

  /* Other classes */
  GameObjectManager::register_class(vm);
  Sector::register_class(vm);
  worldmap::WorldMapSector::register_class(vm);

  /* Global constants */
  vm.setConst<int>("ANCHOR_TOP_LEFT", AnchorPoint::ANCHOR_TOP_LEFT);
  vm.setConst<int>("ANCHOR_TOP", AnchorPoint::ANCHOR_TOP);
  vm.setConst<int>("ANCHOR_TOP_RIGHT", AnchorPoint::ANCHOR_TOP_RIGHT);
  vm.setConst<int>("ANCHOR_LEFT", AnchorPoint::ANCHOR_LEFT);
  vm.setConst<int>("ANCHOR_MIDDLE", AnchorPoint::ANCHOR_MIDDLE);
  vm.setConst<int>("ANCHOR_RIGHT", AnchorPoint::ANCHOR_RIGHT);
  vm.setConst<int>("ANCHOR_BOTTOM_LEFT", AnchorPoint::ANCHOR_BOTTOM_LEFT);
  vm.setConst<int>("ANCHOR_BOTTOM", AnchorPoint::ANCHOR_BOTTOM);
  vm.setConst<int>("ANCHOR_BOTTOM_RIGHT", AnchorPoint::ANCHOR_BOTTOM_RIGHT);

  /* Global functions */
  using namespace scripting;
  vm.addFunc("display", &Globals::display);
  vm.addFunc("print_stacktrace", &Globals::print_stacktrace);
  vm.addFunc("get_current_thread", &Globals::get_current_thread);
  vm.addFunc("is_christmas", &Globals::is_christmas);
  vm.addFunc("start_cutscene", &Globals::start_cutscene);
  vm.addFunc("end_cutscene", &Globals::end_cutscene);
  vm.addFunc("check_cutscene", &Globals::check_cutscene);
  vm.addFunc("wait", &Globals::wait);
  vm.addFunc("wait_for_screenswitch", &Globals::wait_for_screenswitch);
  vm.addFunc("exit_screen", &Globals::exit_screen);
  vm.addFunc("translate", &Globals::translate);
  vm.addFunc("_", &Globals::translate);
  vm.addFunc("translate_plural", &Globals::translate_plural);
  vm.addFunc("__", &Globals::translate_plural);
  vm.addFunc("display_text_file", &Globals::display_text_file);
  vm.addFunc("load_worldmap", &Globals::load_worldmap);
  vm.addFunc("set_next_worldmap", &Globals::set_next_worldmap);
  vm.addFunc("load_level", &Globals::load_level);
  vm.addFunc("import", &Globals::import);
  vm.addFunc("debug_collrects", &Globals::debug_collrects);
  vm.addFunc("debug_show_fps", &Globals::debug_show_fps);
  vm.addFunc("debug_draw_solids_only", &Globals::debug_draw_solids_only);
  vm.addFunc("debug_draw_editor_images", &Globals::debug_draw_editor_images);
  vm.addFunc("debug_worldmap_ghost", &Globals::debug_worldmap_ghost);
  vm.addFunc("set_game_speed", &Globals::set_game_speed);
  vm.addFunc("save_state", &Globals::save_state);
  vm.addFunc("load_state", &Globals::load_state);
  vm.addFunc("play_music", &Globals::play_music);
  vm.addFunc("fade_in_music", &Globals::fade_in_music);
  vm.addFunc("stop_music", &Globals::stop_music);
  vm.addFunc("resume_music", &Globals::resume_music);
  vm.addFunc("pause_music", &Globals::pause_music);
  vm.addFunc("play_sound", &Globals::play_sound);
  vm.addFunc("grease", &Globals::grease);
  vm.addFunc("invincible", &Globals::invincible);
  vm.addFunc("ghost", &Globals::ghost);
  vm.addFunc("mortal", &Globals::mortal);
  vm.addFunc("restart", &Globals::restart);
  vm.addFunc("whereami", &Globals::whereami);
  vm.addFunc("gotoend", &Globals::gotoend);
  vm.addFunc("warp", &Globals::warp);
  vm.addFunc("set_gamma", &Globals::set_gamma);
  vm.addFunc("rand", &Globals::rand);
  vm.addFunc("set_title_frame", &Globals::set_title_frame);

  /* "Level" global functions */
  ssq::Table level = vm.addTable("Level");
  level.addFunc("finish", &Level::finish);
  level.addFunc("has_active_sequence", &Level::has_active_sequence);
  level.addFunc("spawn", &Level::spawn);
  level.addFunc("set_start_point", &Level::set_start_point);
  level.addFunc("set_start_pos", &Level::set_start_pos);
  level.addFunc("set_respawn_point", &Level::set_respawn_point);
  level.addFunc("set_respawn_pos", &Level::set_respawn_pos);
  level.addFunc("flip_vertically", &Level::flip_vertically);
  level.addFunc("toggle_pause", &Level::toggle_pause);
  level.addFunc("pause_target_timer", &Level::pause_target_timer);
  level.addFunc("resume_target_timer", &Level::resume_target_timer);
}

/* EOF */
