//  $Id: main.cpp 3275 2006-04-09 00:32:34Z sommer $
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
#include "msg.hpp"
#include "mainloop.hpp"
#include "worldmap.hpp"

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

  std::auto_ptr<WorldMap> worldmap(new WorldMap());
  worldmap->loadmap(filename);
  main_loop->push_screen(worldmap.release());
}

void load_level(const std::string& filename)
{
  main_loop->push_screen(new GameSession(filename, ST_GL_PLAY));
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

}

