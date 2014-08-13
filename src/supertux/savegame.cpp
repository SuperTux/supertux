//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmx.de>
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

#include "supertux/savegame.hpp"

#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/serialize.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/player_status.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "worldmap/worldmap.hpp"

namespace {

void get_table_entry(HSQUIRRELVM vm, const std::string& name)
{
  sq_pushstring(vm, name.c_str(), -1);
  if(SQ_FAILED(sq_get(vm, -2)))
  {
    throw std::runtime_error("failed to get '" + name + "' table entry");
  }
  else
  {
    // successfully placed result on stack
  } 
}

std::vector<std::string> get_table_keys(HSQUIRRELVM vm)
{
  std::vector<std::string> worlds;

  sq_pushnull(vm);
  while(SQ_SUCCEEDED(sq_next(vm, -2)))
  {
    //here -1 is the value and -2 is the key
    const char* result;
    if(SQ_FAILED(sq_getstring(vm, -2, &result))) 
    {
      std::ostringstream msg;
      msg << "Couldn't get string value for key";
      throw scripting::SquirrelError(vm, msg.str());
    }
    else
    {
      worlds.push_back(result);
    }

    // pops key and val before the next iteration
    sq_pop(vm, 2);
  }

  return worlds;
}

std::vector<LevelState> get_level_states(HSQUIRRELVM vm)
{
  std::vector<LevelState> results;

  sq_pushnull(vm);
  while(SQ_SUCCEEDED(sq_next(vm, -2)))
  {
    //here -1 is the value and -2 is the key
    const char* result;
    if(SQ_FAILED(sq_getstring(vm, -2, &result))) 
    {
      std::ostringstream msg;
      msg << "Couldn't get string value";
      throw scripting::SquirrelError(vm, msg.str());
    }
    else
    {
      LevelState level_state;
      level_state.filename = result;
      scripting::get_bool(vm, "solved", level_state.solved);
      scripting::get_bool(vm, "perfect", level_state.perfect);

      results.push_back(level_state);
    }

    // pops key and val before the next iteration
    sq_pop(vm, 2);
  }

  return results;
}

} // namespace

Savegame::Savegame(const std::string& filename) :
  m_filename(filename),
  m_player_status(new PlayerStatus)
{
}

Savegame::~Savegame()
{
}

void
Savegame::load()
{
  if (m_filename.empty())
  {
    log_debug << "no filename set for savegame, skipping load" << std::endl;
    return;
  }

  if(!PHYSFS_exists(m_filename.c_str()))
  {
    log_info << m_filename << ": doesn't exist, not loading state" << std::endl;
  }
  else
  {
    log_debug << "loading savegame from " << m_filename << std::endl;

    try
    {
      HSQUIRRELVM vm = scripting::global_vm;

      lisp::Parser parser;
      const lisp::Lisp* root = parser.parse(m_filename);

      const lisp::Lisp* lisp = root->get_lisp("supertux-savegame");
      if(lisp == NULL)
      {
        throw std::runtime_error("file is not a supertux-savegame file");
      }
      else
      {
        int version = 1;
        lisp->get("version", version);
        if(version != 1)
        {
          throw std::runtime_error("incompatible savegame version");
        }
        else
        {
          const lisp::Lisp* tux = lisp->get_lisp("tux");
          if(tux == NULL)
          {
            throw std::runtime_error("No tux section in savegame");
          }
          {
            m_player_status->read(*tux);
          }

          const lisp::Lisp* state = lisp->get_lisp("state");
          if(state == NULL)
          {
            throw std::runtime_error("No state section in savegame");
          }
          else
          {
            // delete existing state table, if it exists
            sq_pushroottable(vm);
            sq_pushstring(vm, "state", -1);
            if(SQ_FAILED(sq_deleteslot(vm, -2, SQFalse)))
              sq_pop(vm, 1);

            // create a new empty state table
            sq_pushstring(vm, "state", -1);
            sq_newtable(vm);
            scripting::load_squirrel_table(vm, -1, *state);
            if(SQ_FAILED(sq_createslot(vm, -3)))
              throw std::runtime_error("Couldn't create state table");
            sq_pop(vm, 1);
          }
        }
      }
    }
    catch(const std::exception& e)
    {
      log_fatal << "Couldn't load savegame: " << e.what() << std::endl;
    }
  }
}

void
Savegame::save()
{
  if (m_filename.empty())
  {
    log_debug << "no filename set for savegame, skipping save" << std::endl;
    return;
  }

  log_debug << "saving savegame to " << m_filename << std::endl;

  { // make sure the savegame directory exists
    std::string dirname = FileSystem::dirname(m_filename);
    if(!PHYSFS_exists(dirname.c_str()))
    {
      if(!PHYSFS_mkdir(dirname.c_str()))
      {
        std::ostringstream msg;
        msg << "Couldn't create directory for savegames '"
            << dirname << "': " <<PHYSFS_getLastError();
        throw std::runtime_error(msg.str());
      }
    }

    if(!PHYSFS_isDirectory(dirname.c_str()))
    {
      std::ostringstream msg;
      msg << "Savegame path '" << dirname << "' is not a directory";
      throw std::runtime_error(msg.str());
    }
  }

  HSQUIRRELVM vm = scripting::global_vm;

  lisp::Writer writer(m_filename);

  writer.start_list("supertux-savegame");
  writer.write("version", 1);

  using namespace worldmap;
  if(WorldMap::current() != NULL)
  {
    std::ostringstream title;
    title << WorldMap::current()->get_title();
    title << " (" << WorldMap::current()->solved_level_count()
          << "/" << WorldMap::current()->level_count() << ")";
    writer.write("title", title.str());
  }

  writer.start_list("tux");
  m_player_status->write(writer);
  writer.end_list("tux");

  writer.start_list("state");

  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  if(SQ_SUCCEEDED(sq_get(vm, -2)))
  {
    scripting::save_squirrel_table(vm, -1, writer);
    sq_pop(vm, 1);
  }
  sq_pop(vm, 1);
  writer.end_list("state");

  writer.end_list("supertux-savegame");
}

std::vector<std::string>
Savegame::get_worldmaps()
{
  std::vector<std::string> worlds;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_table_entry(vm, "worlds");
    worlds = get_table_keys(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return worlds;
}

WorldmapState
Savegame::get_worldmap_state(const std::string& name)
{
  WorldmapState result;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_table_entry(vm, "worlds");
    get_table_entry(vm, name);
    get_table_entry(vm, "levels");

    result.level_states = get_level_states(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return result;
}

std::vector<std::string>
Savegame::get_levelsets()
{
  std::vector<std::string> results;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_table_entry(vm, "levelsets");
    results = get_table_keys(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return results;
}

LevelsetState
Savegame::get_levelset_state(const std::string& name)
{
  LevelsetState result;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_table_entry(vm, "levelsets");
    get_table_entry(vm, name);
    get_table_entry(vm, "levels");

    result.level_states = get_level_states(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return result;
}

/* EOF */
