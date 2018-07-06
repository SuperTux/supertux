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
#include "math/random_generator.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "physfs/ifile_stream.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/shrinkfade.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/tile.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

#include "scripting/squirrel_util.hpp"
#include "scripting/time_scheduler.hpp"

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
  sq_pushobject(vm, vm_to_object(vm));
  return 1;
}

SQInteger is_christmas(HSQUIRRELVM vm)
{
    return g_config->christmas_mode;
}

void wait(HSQUIRRELVM vm, float seconds)
{
  TimeScheduler::instance->schedule_thread(vm, game_time + seconds);
}

void wait_for_screenswitch(HSQUIRRELVM vm)
{
  ScreenManager::current()->m_waiting_threads.add(vm);
}

void exit_screen()
{
  ScreenManager::current()->pop_screen();
}

void fadeout_screen(float seconds)
{
  ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeOut(seconds)));
}

void shrink_screen(float dest_x, float dest_y, float seconds)
{
  ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new ShrinkFade(Vector(dest_x, dest_y), seconds)));
}

void abort_screenfade()
{
  ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>());
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
  ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new TextScroller(filename)));
}

void load_worldmap(const std::string& filename)
{
  using namespace worldmap;

  if (!WorldMap::current())
  {
    throw std::runtime_error("Can't start Worldmap without active WorldMap");
  }
  else
  {
    ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new WorldMap(filename, WorldMap::current()->get_savegame())));
  }
}

void set_next_worldmap(const std::string& dirname, const std::string& spawnpoint)
{
  GameManager::current()->set_next_worldmap(dirname, spawnpoint);
}

void load_level(const std::string& filename)
{
  if (!GameSession::current())
  {
    throw std::runtime_error("Can't start level without active level.");
  }
  else
  {
    ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new GameSession(filename, GameSession::current()->get_savegame())));
  }
}

void import(HSQUIRRELVM vm, const std::string& filename)
{
  IFileStream in(filename);
  scripting::compile_and_run(vm, in, filename);
}

void debug_collrects(bool enable)
{
  ::Sector::show_collrects = enable;
}

void debug_show_fps(bool enable)
{
  g_config->show_fps = enable;
}

void debug_draw_solids_only(bool enable)
{
  ::Sector::draw_solids_only = enable;
}

void debug_draw_editor_images(bool enable)
{
  Tile::draw_editor_images = enable;
}

void debug_worldmap_ghost(bool enable)
{
  auto worldmap = worldmap::WorldMap::current();

  if(worldmap == NULL)
    throw std::runtime_error("Can't change ghost mode without active WorldMap");

  worldmap->get_tux()->set_ghost_mode(enable);
}

void save_state()
{
  auto worldmap = worldmap::WorldMap::current();

  if (!worldmap)
  {
    throw std::runtime_error("Can't save state without active Worldmap");
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
    throw std::runtime_error("Can't save state without active Worldmap");
  }
  else
  {
    worldmap->load_state();
  }
}

// not added to header, function to only be used by others
// in this file
bool validate_sector_player()
{
  if (::Sector::current() == 0)
  {
    log_info << "No current sector." << std::endl;
    return false;
  }

  if (::Sector::current()->player == 0)
  {
    log_info << "No player." << std::endl;
    return false;
  }
  return true;
}

void play_music(const std::string& filename)
{
  SoundManager::current()->play_music(filename);
}

void play_sound(const std::string& filename)
{
  SoundManager::current()->play(filename);
}

void grease()
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player; // scripting::Player != ::Player
  tux->get_physic().set_velocity_x(tux->get_physic().get_velocity_x()*3);
}

void invincible()
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player;
  tux->invincible_timer.start(10000);
}

void ghost()
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player;
  tux->set_ghost_mode(true);
}

void mortal()
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player;
  tux->invincible_timer.stop();
  tux->set_ghost_mode(false);
}

void restart()
{
  auto session = GameSession::current();
  if (session == 0)
  {
    log_info << "No game session" << std::endl;
    return;
  }
  session->restart_level();
}

void whereami()
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player;
  log_info << "You are at x " << ((int) tux->get_pos().x) << ", y " << ((int) tux->get_pos().y) << std::endl;
}

void gotoend()
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player;
  tux->move(Vector(
              (::Sector::current()->get_width()) - (SCREEN_WIDTH*2), 0));
  ::Sector::current()->camera->reset(
    Vector(tux->get_pos().x, tux->get_pos().y));
}

void warp(float offset_x, float offset_y)
{
  if (!validate_sector_player()) return;
  auto tux = ::Sector::current()->player;
  tux->move(Vector(
              tux->get_pos().x + (offset_x*32), tux->get_pos().y - (offset_y*32)));
  ::Sector::current()->camera->reset(
    Vector(tux->get_pos().x, tux->get_pos().y));
}

void camera()
{
  if (!validate_sector_player()) return;
  auto& cam_pos = ::Sector::current()->camera->get_translation();
  log_info << "Camera is at " << cam_pos.x << "," << cam_pos.y << std::endl;
}

void set_gamma(float gamma)
{
  VideoSystem::current()->get_renderer().set_gamma(gamma);
}

void quit()
{
  ScreenManager::current()->quit();
}

int rand()
{
  return gameRandom.rand();
}

void set_game_speed(float speed)
{
  ::g_game_speed = speed;
}

void record_demo(const std::string& filename)
{
  if (GameSession::current() == 0)
  {
    log_info << "No game session" << std::endl;
    return;
  }
  GameSession::current()->restart_level();
  GameSession::current()->record_demo(filename);
}

void play_demo(const std::string& filename)
{
  auto session = GameSession::current();
  if (session == 0)
  {
    log_info << "No game session" << std::endl;
    return;
  }
  // Reset random seed
  g_config->random_seed = session->get_demo_random_seed(filename);
  g_config->random_seed = gameRandom.srand(g_config->random_seed);
  session->restart_level();
  session->play_demo(filename);
}

}

/* EOF */
