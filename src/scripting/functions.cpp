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

#include "scripting/functions.hpp"

#include "audio/sound_manager.hpp"
#include "math/random.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "physfs/ifile_stream.hpp"
#include "supertux/console.hpp"
#include "supertux/debug.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/level.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/shrinkfade.hpp"
#include "supertux/textscroller_screen.hpp"
#include "supertux/tile.hpp"
#include "supertux/title_screen.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

namespace {

// Not added to header, function to only be used by others
// in this file.
bool validate_sector_player()
{
  if (::Sector::current() == nullptr)
  {
    log_info << "No current sector." << std::endl;
    return false;
  }

  return true;
}

} // namespace

namespace scripting {

SQInteger display(HSQUIRRELVM vm)
{
  ConsoleBuffer::output << squirrel2string(vm, -1) << std::endl;
  return 0;
}

void print_stacktrace(HSQUIRRELVM vm)
{
  print_squirrel_stack(vm);
}

SQInteger get_current_thread(HSQUIRRELVM vm)
{
  sq_pushthread(vm, vm);
  return 1;
}

bool is_christmas()
{
  return g_config->christmas_mode;
}

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

void wait(HSQUIRRELVM vm, float seconds)
{
  auto session = GameSession::current();

  if(session && session->get_current_level().m_skip_cutscene)
  {
    if (auto squirrelenv = static_cast<SquirrelEnvironment*>(sq_getforeignptr(vm)))
    {
      // Wait anyways, to prevent scripts like `while (true) {wait(0.1); ...}`.
      squirrelenv->wait_for_seconds(vm, 0);
    }
    else if (auto squirrelvm = static_cast<SquirrelVirtualMachine*>(sq_getsharedforeignptr(vm)))
    {
      squirrelvm->wait_for_seconds(vm, 0);
    }
    else
    {
      log_warning << "wait(): no VM or environment available\n";
    }
  }
  else if(session && session->get_current_level().m_is_in_cutscene)
  {
    if (auto squirrelenv = static_cast<SquirrelEnvironment*>(sq_getforeignptr(vm)))
    {
      // Wait anyways, to prevent scripts like `while (true) {wait(0.1); ...}` from freezing the game.
      squirrelenv->skippable_wait_for_seconds(vm, seconds);
      //session->set_scheduler(squirrelenv->get_scheduler());
    }
    else if (auto squirrelvm = static_cast<SquirrelVirtualMachine*>(sq_getsharedforeignptr(vm)))
    {
      squirrelvm->skippable_wait_for_seconds(vm, seconds);
      //session->set_scheduler(squirrelvm->get_scheduler());
    }
    else
    {
      log_warning << "wait(): no VM or environment available\n";
    }
  }
  else
  {
    if (auto squirrelenv = static_cast<SquirrelEnvironment*>(sq_getforeignptr(vm)))
    {
      squirrelenv->wait_for_seconds(vm, seconds);
    }
    else if (auto squirrelvm = static_cast<SquirrelVirtualMachine*>(sq_getsharedforeignptr(vm)))
    {
      squirrelvm->wait_for_seconds(vm, seconds);
    }
    else
    {
      log_warning << "wait(): no VM or environment available\n";
    }
  }
}

void wait_for_screenswitch(HSQUIRRELVM vm)
{
  auto squirrelvm = static_cast<SquirrelVirtualMachine*>(sq_getsharedforeignptr(vm));
  //auto squirrelenv = static_cast<SquirrelEnvironment*>(sq_getforeignptr(vm));
  squirrelvm->wait_for_screenswitch(vm);
}

void exit_screen()
{
  ScreenManager::current()->pop_screen();
}

std::string translate(const std::string& text)
{
  return g_dictionary_manager->get_dictionary().translate(text);
}

std::string _(const std::string& text)
{
  return translate(text);
}

std::string translate_plural(const std::string& text, const std::string& text_plural, int num)
{
  return g_dictionary_manager->get_dictionary().translate_plural(text, text_plural, num);
}

std::string __(const std::string& text, const std::string& text_plural, int num)
{
  return translate_plural(text, text_plural, num);
}

void display_text_file(const std::string& filename)
{
  ScreenManager::current()->push_screen(std::make_unique<TextScrollerScreen>(filename));
}

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

void set_next_worldmap(const std::string& dirname, const std::string& sector, const std::string& spawnpoint)
{
  GameManager::current()->set_next_worldmap(dirname, sector, spawnpoint);
}

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

void import(HSQUIRRELVM vm, const std::string& filename)
{
  IFileStream in(filename);
  compile_and_run(vm, in, filename);
}

void debug_collrects(bool enable)
{
  g_debug.show_collision_rects = enable;
}

void debug_show_fps(bool enable)
{
  g_config->show_fps = enable;
}

void debug_draw_solids_only(bool enable)
{
  ::Sector::s_draw_solids_only = enable;
}

void debug_draw_editor_images(bool enable)
{
  Tile::draw_editor_images = enable;
}

void debug_worldmap_ghost(bool enable)
{
  auto worldmap_sector = worldmap::WorldMapSector::current();

  if (worldmap_sector == nullptr)
    throw std::runtime_error("Can't change ghost mode without active WorldMapSector.");

  auto& tux = worldmap_sector->get_singleton_by_type<worldmap::Tux>();
  tux.set_ghost_mode(enable);
}

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

void play_music(const std::string& filename)
{
  SoundManager::current()->play_music(filename);
}

void stop_music(float fadetime)
{
  SoundManager::current()->stop_music(fadetime);
}

void fade_in_music(const std::string& filename, float fadetime)
{
  SoundManager::current()->play_music(filename, fadetime);
}

void resume_music(float fadetime)
{
  SoundManager::current()->resume_music(fadetime);
}

void pause_music(float fadetime)
{
  SoundManager::current()->pause_music(fadetime);
}

void play_sound(const std::string& filename)
{
  SoundManager::current()->play(filename);
}

void grease()
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]); // scripting::Player != ::Player
  tux.get_physic().set_velocity_x(tux.get_physic().get_velocity_x()*3);
}

void invincible()
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.m_invincible_timer.start(10000);
}

void ghost()
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.set_ghost_mode(true);
}

void mortal()
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.m_invincible_timer.stop();
  tux.set_ghost_mode(false);
}

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

void whereami()
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  log_info << "You are at x " << (static_cast<int>(tux.get_pos().x)) << ", y " << (static_cast<int>(tux.get_pos().y)) << std::endl;
}

void gotoend()
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.move(Vector(
              (::Sector::get().get_width()) - (static_cast<float>(SCREEN_WIDTH) * 2.0f), 0));
  ::Sector::get().get_camera().reset(
    Vector(tux.get_pos().x, tux.get_pos().y));
}

void warp(float offset_x, float offset_y)
{
  if (!validate_sector_player()) return;
  // FIXME: This only has effect on the first player.
  ::Player& tux = *(::Sector::get().get_players()[0]);
  tux.move(Vector(
              tux.get_pos().x + (offset_x*32), tux.get_pos().y - (offset_y*32)));
  ::Sector::get().get_camera().reset(
    Vector(tux.get_pos().x, tux.get_pos().y));
}

void camera()
{
  if (!validate_sector_player()) return;
  const auto& cam_pos = ::Sector::get().get_camera().get_translation();
  log_info << "Camera is at " << cam_pos.x << "," << cam_pos.y << std::endl;
}

void set_gamma(float gamma)
{
  VideoSystem::current()->set_gamma(gamma);
}

int rand()
{
  return gameRandom.rand();
}

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

}

/* EOF */
