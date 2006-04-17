//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include <config.h>

#include <memory>
#include <stdio.h>
#include <string>
#include <squirrel.h>
#include <sqstdio.h>
#include "textscroller.hpp"
#include "functions.hpp"
#include "game_session.hpp"
#include "tinygettext/tinygettext.hpp"
#include "physfs/physfs_stream.hpp"
#include "script_manager.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "mainloop.hpp"
#include "worldmap/worldmap.hpp"
#include "world.hpp"
#include "sector.hpp"
#include "gameconfig.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "main.hpp"
#include "object/camera.hpp"
#include "flip_level_transformer.hpp"

#include "squirrel_error.hpp"
#include "wrapper_util.hpp"

namespace Scripting
{

int display(HSQUIRRELVM vm)
{
  Console::output << squirrel2string(vm, -1) << std::endl;
  return 0;
}

void wait(HSQUIRRELVM vm, float seconds)
{
  SQUserPointer ptr = sq_getforeignptr(vm);
  ScriptManager* script_manager = reinterpret_cast<ScriptManager*> (ptr);
  script_manager->set_wakeup_event(vm, ScriptManager::TIME, seconds);
}

void wait_for_screenswitch(HSQUIRRELVM vm)
{
  SQUserPointer ptr = sq_getforeignptr(vm);
  ScriptManager* script_manager = reinterpret_cast<ScriptManager*> (ptr);
  script_manager->set_wakeup_event(vm, ScriptManager::SCREEN_SWITCHED);
}

void exit_screen()
{
  main_loop->exit_screen();
}

std::string translate(const std::string& text)
{
  return dictionary_manager.get_dictionary().translate(text);
}

void display_text_file(const std::string& filename)
{
  main_loop->push_screen(new TextScroller(filename));
}

void load_worldmap(const std::string& filename)
{
  using namespace WorldMapNS;

  main_loop->push_screen(new WorldMap(filename));
}

void load_level(const std::string& filename)
{
  main_loop->push_screen(new GameSession(filename));
}

static SQInteger squirrel_read_char(SQUserPointer file)
{
  std::istream* in = reinterpret_cast<std::istream*> (file);
  char c = in->get();
  if(in->eof())
    return 0;

  return c;
}


void import(HSQUIRRELVM vm, const std::string& filename)
{
  IFileStream in(filename);
    
  if(SQ_FAILED(sq_compile(vm, squirrel_read_char, &in,
          filename.c_str(), SQTrue)))
    throw SquirrelError(vm, "Couldn't parse script");
    
  sq_pushroottable(vm);
  if(SQ_FAILED(sq_call(vm, 1, SQFalse))) {
    sq_pop(vm, 1);
    throw SquirrelError(vm, "Couldn't execute script");
  }
  sq_pop(vm, 1);
}

void add_key(int new_key)
{
  player_status->set_keys(new_key);
}

void debug_collrects(bool enable)
{
  Sector::show_collrects = enable;
}

void debug_draw_fps(bool enable)
{
  config->show_fps = enable;
}

void debug_draw_solids_only(bool enable)
{
  Sector::draw_solids_only = enable;
}

void save_state()
{
  if(World::current() == NULL)
    throw std::runtime_error("Can't save state without active World");

  World::current()->save_state();
}

// not added to header, function to only be used by others
// in this file
bool validate_sector_player()
{
  if (Sector::current() == 0)
  {
    log_info << "No current sector." << std::endl;
	return false;
  }

  if (Sector::current()->player == 0)
  {
    log_info << "No player." << std::endl;
	return false;
  }
  return true;
}

void grease()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player; // Scripting::Player != ::Player
  tux->physic.set_velocity_x(tux->physic.get_velocity_x()*3);
}

void invincible()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player;
  tux->invincible_timer.start(10000);
}

void mortal()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player;
  tux->invincible_timer.stop();
}

void shrink()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player;
  tux->kill(tux->SHRINK);
}

void kill()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player;
  tux->kill(tux->KILL);
}

void restart()
{
  if (GameSession::current() == 0)
  {
    log_info << "No game session" << std::endl;
    return;
  }
  GameSession::current()->restart_level();
}

void whereami()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player;
  log_info << "You are at x " << tux->get_pos().x << ", y " << tux->get_pos().y << std::endl;
}

void gotoend()
{
  if (!validate_sector_player()) return;
  ::Player* tux = Sector::current()->player;
  tux->move(Vector(
          (Sector::current()->solids->get_width()*32) - (SCREEN_WIDTH*2), 0));
  Sector::current()->camera->reset(
        Vector(tux->get_pos().x, tux->get_pos().y));
}

void camera()
{
  if (!validate_sector_player()) return;
  log_info << "Camera is at " << Sector::current()->camera->get_translation().x << "," << Sector::current()->camera->get_translation().y << std::endl;
}

void quit()
{
  main_loop->quit();
}

}

